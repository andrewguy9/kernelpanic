#!/usr/bin/python

import os
import sys
import signal
import fnmatch
import subprocess

import argparse

parser = argparse.ArgumentParser(description='Test wrapper which collects stats and stacks')
timing_group = parser.add_mutually_exclusive_group()
timing_group.add_argument('--timeout', dest='timeout', type=int)
timing_group.add_argument('--until', dest='until', type=int)
parser.add_argument('--debugger', type=str, help='Which debugger to use')
parser.add_argument('--coredir', type=str, help='Where to find core files')
parser.add_argument('jobid', type=str, help='id of the test pass')
parser.add_argument('branch', type=str, help='which branch is being tested')
parser.add_argument('commit', type=str, help='which commit is being tested')
parser.add_argument('dirty', type=str, help='which files are dirty')
parser.add_argument('testarg', type=str, nargs='+', help='test arguments')
args = parser.parse_args()

child_args = args.testarg
test_name = os.path.normpath(child_args[0])
test_path = os.path.join('.', test_name)
child_args[0] = test_path
debugger = args.debugger
coredir = args.coredir

def exit_with_msg(test, pid, msg, status, usage, stack=""):
    output = "Test %s(%s)... %s" % (test_name, pid, msg)
    if stack:
        output += "\n"
        output += stack
    print output
    user_time = usage.ru_utime
    sys_time = usage.ru_stime
    total_time = user_time + sys_time
    with open("perf.csv", "a+") as data:
        print >> data, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s" % (
                args.jobid,
                args.branch,
                args.commit,
                args.dirty,
                test,
                total_time,
                user_time,
                sys_time,
                status,
                msg)
    exit(status)

def find_core(pid):
    names = os.listdir(coredir)
    matches = fnmatch.filter(names, "*%s*" % pid)
    if len(matches) == 0:
        return None
    return os.path.normpath(os.path.join(coredir, matches[0]))

def move_core(dst_core, src_core):
    if src_core is None:
        return None
    if src_core != dst_core:
        os.rename(src_core, dst_core)
    return dst_core

def get_stack(program, core):
    if core is None:
        return None
    if debugger == "gdb":
        debug_cmd = ["gdb", "--quiet", "--command", "./get_stack.gdb", program, core]
    elif debugger == "lldb":
        debug_cmd = ["lldb", "--core", core, "--source", "./get_stack.gdb"]
    else:
        raise ValueError("Unrecognized debugger")
    stack  = "-" * 80 + "\n"
    stack += " ".join(debug_cmd) + "\n"
    stack += subprocess.check_output(debug_cmd)
    stack += "-" * 80
    return stack

child = os.fork()
if child == 0: # child
    os.execvp(test_path, child_args)
else: # parent
    def alarm_handler(signum, frame):
        os.kill(child, signal.SIGINT)
        (pid, status, usage) = os.wait4(child, 0)
        if args.timeout:
            result = "TIMEOUT"
        elif args.until:
            result = "SUCCESS"
        exit_with_msg(test_name, child, result, status, usage)
    signal.signal(signal.SIGALRM, alarm_handler)
    if args.timeout:
        signal.alarm(args.timeout)
    elif args.until:
        signal.alarm(args.until)
    (pid, status, usage) = os.wait4(child, 0)
    signal.alarm(0)
    test_pid = pid
    if not (status == 0 or status == 1):
        src_core = find_core(test_pid)
        dst_core = os.path.normpath("./%s.%s.core" % (test_name, test_pid))
        core_path = move_core(dst_core, src_core)
        stack = get_stack(test_name, core_path)
        exit_with_msg(test_name, test_pid, "FAILED", status, usage, stack)
    exit_with_msg(test_name, test_pid, "SUCCESS", status, usage)
