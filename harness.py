#!/usr/bin/env python3

import argparse
import fnmatch
import os
import signal
import subprocess
import sys
import time

from test_tools import results_db


def parse_args():
    parser = argparse.ArgumentParser(description='Test wrapper which collects stats and stacks')
    timing_group = parser.add_mutually_exclusive_group()
    timing_group.add_argument('--timeout', dest='timeout', type=int)
    timing_group.add_argument('--until', dest='until', type=int)
    parser.add_argument('--runs', dest='runs', type=int, default=1)
    parser.add_argument('--batch', dest='batch', type=int, default=1)
    parser.add_argument('--each', dest='each', action='store_true',
            help='treat each testarg as a separate test command')
    parser.add_argument('--debugger', dest='debugger', type=str, help='Which debugger to use')
    parser.add_argument('coresdir', type=str, help='path to cores directory')
    parser.add_argument('jobid', type=str, help='id of the test pass')
    parser.add_argument('branch', type=str, help='which branch is being tested')
    parser.add_argument('commit', type=str, help='which commit is being tested')
    parser.add_argument('dirty', type=str, help='which files are dirty')
    parser.add_argument('testarg', type=str, nargs='+', help='test arguments')
    args = parser.parse_args()
    if args.runs < 1:
        parser.error("--runs must be at least 1")
    if args.batch < 1:
        parser.error("--batch must be at least 1")
    return args


def shell_status(status):
    if os.WIFEXITED(status):
        return os.WEXITSTATUS(status)
    if os.WIFSIGNALED(status):
        return 128 + os.WTERMSIG(status)
    return 1


def result_exit_status(msg, status):
    if msg == "SUCCESS":
        return 0
    result = shell_status(status)
    if result == 0:
        return 1
    return result


def report_result(args, test_name, test, pid, msg, status, usage, stack=""):
    output = "Test %s(%s)... %s" % (test_name, pid, msg)
    if stack:
        output += "\n"
        output += stack
    print(output)
    user_time = usage.ru_utime
    sys_time = usage.ru_stime
    total_time = user_time + sys_time
    with results_db.connect() as connection:
        connection.execute(
            """
            INSERT INTO perf (
              job,
              branch,
              commit_hash,
              dirty,
              test,
              real,
              user,
              sys,
              status,
              result
            )
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            """,
            (
                args.jobid,
                args.branch,
                args.commit,
                args.dirty,
                test,
                total_time,
                user_time,
                sys_time,
                status,
                msg))
    return result_exit_status(msg, status)


def find_core(coresdir, pid):
    try:
        names = os.listdir(coresdir)
    except OSError:
        return None
    matches = fnmatch.filter(names, "*%s*" % pid)
    if len(matches) == 0:
        return None
    return os.path.normpath(os.path.join(coresdir, matches[0]))


def move_core(dst_core, src_core):
    if src_core is None:
        return None
    if src_core != dst_core:
        os.rename(src_core, dst_core)
    return dst_core


def get_stack(debugger, program, core):
    if core is None:
        return None
    if debugger == "gdb":
        debug_cmd = ["gdb", "--quiet", "--batch", "--command", "./get_stack.gdb", program, core]
    elif debugger == "lldb":
        debug_cmd = ["lldb", "--batch", "--core", core, "--source", "./get_stack.gdb"]
    else:
        return "Unrecognized debugger: %s" % debugger
    stack  = "-" * 80 + "\n"
    stack += " ".join(debug_cmd) + "\n"
    try:
        result = subprocess.run(
            debug_cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            timeout=10,
            check=False)
        stack += result.stdout
    except subprocess.TimeoutExpired as error:
        stack += error.stdout or ""
        stack += "Debugger timed out while collecting stack.\n"
    stack += "-" * 80
    return stack


def run_child(test_name, test_path, child_args):
    childenv = dict(os.environ)
    childenv['LLVM_PROFILE_FILE'] = "%s.%%p.profraw" % test_name
    try:
        os.execvpe(test_path, child_args, childenv)
    except OSError as error:
        print("Failed to exec %s: %s" % (test_path, error), file=sys.stderr)
        os._exit(127)


def start_test(child_args):
    child_args = list(child_args)
    test_name = os.path.normpath(child_args[0])
    test_path = os.path.abspath(os.path.join('.', test_name))
    child_args[0] = test_path

    child = os.fork()
    if child == 0:
        run_child(test_name, test_path, child_args)

    return {
        "pid": child,
        "test_name": test_name,
        "start": time.monotonic(),
        "expired": False,
    }


def finish_test(args, test, status, usage, result=None):
    test_name = test["test_name"]
    test_pid = test["pid"]

    if result is not None:
        return report_result(args, test_name, test_name, test_pid, result, status, usage)

    if not (status == 0 or status == 1):
        src_core = find_core(args.coresdir, test_pid)
        dst_core = os.path.normpath("./%s.%s.core" % (test_name, test_pid))
        core_path = move_core(dst_core, src_core)
        stack = get_stack(args.debugger, test_name, core_path)
        return report_result(args, test_name, test_name, test_pid, "FAILED", status, usage, stack)
    return report_result(args, test_name, test_name, test_pid, "SUCCESS", status, usage)


def is_executable_path(path):
    return os.path.isfile(path) and os.access(path, os.X_OK)


def split_test_commands(testargs, force_each):
    if force_each:
        return [[arg] for arg in testargs]
    if len(testargs) > 1 and all(is_executable_path(arg) for arg in testargs):
        return [[arg] for arg in testargs]
    return [testargs]


def repeated_test_commands(testargs, runs, force_each):
    commands = split_test_commands(testargs, force_each)
    return commands * runs


def timeout_result(args):
    if args.timeout:
        return "TIMEOUT"
    if args.until:
        return "SUCCESS"
    return None


def time_limit(args):
    if args.timeout:
        return args.timeout
    if args.until:
        return args.until
    return None


def wait_for_expired_test(args, running, pid):
    test = running.pop(pid)
    test["expired"] = True
    os.kill(pid, signal.SIGINT)
    waited_pid, status, usage = os.wait4(pid, 0)
    if waited_pid != pid:
        raise RuntimeError("wait4 returned unexpected pid")
    return finish_test(args, test, status, usage, timeout_result(args))


def reap_finished_tests(args, running):
    status = 0
    while running:
        try:
            pid, wait_status, usage = os.wait4(-1, os.WNOHANG)
        except ChildProcessError:
            break
        if pid == 0:
            break
        test = running.pop(pid)
        result = finish_test(args, test, wait_status, usage)
        if result != 0:
            status = result
    return status


def reap_expired_tests(args, running):
    limit = time_limit(args)
    if limit is None:
        return 0

    status = 0
    now = time.monotonic()
    for pid, test in list(running.items()):
        if now - test["start"] >= limit:
            result = wait_for_expired_test(args, running, pid)
            if result != 0:
                status = result
    return status


def run_tests(args):
    pending = list(repeated_test_commands(args.testarg, args.runs, args.each))
    running = {}
    status = 0

    while pending or running:
        while pending and len(running) < args.batch:
            test = start_test(pending.pop(0))
            running[test["pid"]] = test

        result = reap_finished_tests(args, running)
        if result != 0:
            status = result

        result = reap_expired_tests(args, running)
        if result != 0:
            status = result

        if running:
            time.sleep(0.05)

    return status


def main():
    args = parse_args()
    # TODO: Consider normalizing aggregate multi-test failures to 1.
    # For now, preserve child-like exit status behavior during the
    # Python 2 to Python 3 conversion.
    status = run_tests(args)
    sys.exit(status)


if __name__ == "__main__":
    main()
