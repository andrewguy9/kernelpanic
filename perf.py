#!/usr/bin/python

import os
import sys
import signal
import fnmatch
import subprocess

import argparse

parser = argparse.ArgumentParser(description='Test wrapper which collects stats and stacks.')
timing_group = parser.add_mutually_exclusive_group()
timing_group.add_argument('--timeout', dest='timeout', type=int)
timing_group.add_argument('--until', dest='until', type=int)
parser.add_argument('jobid', type=str, help='id of the test pass')
parser.add_argument('branch', type=str, help='which branch is being tested')
parser.add_argument('commit', type=str, help='which commit is being tested')
parser.add_argument('dirty', type=str, help='which files are dirty.')
parser.add_argument('testarg', type=str, nargs='+', help='test arguments')
args = parser.parse_args()

child_args = args.testarg
test_name = child_args[0]
test_path = "./"+test_name
child_args[0] = test_path

def exit_with_msg(test,pid, msg, status, stack=""):
    output = "Test %s(%s)... %s" % (test_name, pid, msg)
    if stack:
        output += "\n"
        output += stack
    print output
    exit(status)

def find_core(pid):
    os.listdir("/cores")
    names = os.listdir("/cores") #TODO need to get passed coresdir.
    matches = fnmatch.filter(names, "*%s*" % pid)
    if len(matches) == 0:
        return None
    return os.path.join("/cores", matches[0]) #TODO use coredir var.

def move_core(dst_core, src_core):
    if src_core is None:
        return None
    if src_core != dst_core:
        os.rename(src_core, dst_core)
    return dst_core

def get_stack(program, core):
    debugger = "lldb"
    if core is None:
        return None
    #TODO i need a debugger var.
    if debugger == "gdb":
        debug_cmd = ["gdb", "--command", "./get_stack.gdb", program, core]
    elif debugger == "lldb":
        debug_cmd = ["lldb", "--core", core, "--source", "./get_stack.gdb"]
    else:
        raise ValueError("Unrecognized debugger")
    stack  = "-" * 80 + "\n"
    stack += " ".join(debug_cmd) + "\n"
    stack += subprocess.check_output(debug_cmd)
    stack += "-" * 80
    return stack

def get_coverage(program, pid):
    perf_command = ["./coverage.sh", str(program), str(pid)]
    report = subprocess.check_output(perf_command)
    return report

child = os.fork()
if child == 0: # child
    os.execvp(test_path, child_args)
else: # parent
    def alarm_handler(signum, frame):
        if args.timeout:
            os.kill(child, signal.SIGINT)
            (pid, status, usage) = os.wait4(child, 0)
            get_coverage(test_name, pid)
            exit_with_msg(test_name, child, "TIMEOUT", 1)
        elif args.until:
            os.kill(child, signal.SIGINT)
            (pid, status, usage) = os.wait4(child, 0)
            get_coverage(test_name, pid)
            exit_with_msg(test_name, child, "SUCCESS", 0)
    signal.signal(signal.SIGALRM, alarm_handler)
    if args.timeout:
        signal.alarm(args.timeout)
    elif args.until:
        signal.alarm(args.until)
    (pid, status, usage) = os.wait4(child, 0)
    signal.alarm(0)
    test_pid = pid
    user_time = usage.ru_utime
    sys_time = usage.ru_stime
    total_time = user_time + sys_time
    get_coverage(test_name, test_pid)
    if status != 0:
        src_core = find_core(test_pid)
        dst_core = "./%s.%s.core" % (test_name, test_pid)
        core_path = move_core(dst_core, src_core)
        stack = get_stack(test_name, core_path)
        exit_with_msg(test_name, test_pid, "FAILED", status, stack)
    with open("perf.csv", "a+") as data:
        print >> data, "%s,%s,%s,%s,%s,%s,%s,%s" % (args.jobid,args.branch,args.commit,args.dirty,test_name,total_time, user_time, sys_time)
        exit_with_msg(test_name, test_pid, "SUCCESS", status)
