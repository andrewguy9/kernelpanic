import argparse
import platform
import shutil
import subprocess
import sys
import uuid


USAGE = "%(prog)s [--runs i] [--batch i] [--time i] [--debugger s] [--coredir s] [--jobid s] [--help] <test> ..."


def default_debugger():
    if shutil.which("gdb"):
        return "gdb"
    if shutil.which("lldb"):
        return "lldb"
    return None


def default_coredir():
    system = platform.system()
    if system == "Darwin":
        return "/cores"
    if system == "Linux":
        return "."
    return None


def parse_args(default_timeout):
    parser = argparse.ArgumentParser(usage=USAGE, add_help=False)
    parser.add_argument("--runs", type=int, default=1)
    parser.add_argument("--batch", type=int, default=1)
    parser.add_argument("--time", type=int, default=default_timeout)
    parser.add_argument("--debugger", default=default_debugger())
    parser.add_argument("--coredir", default=default_coredir())
    parser.add_argument("--jobid", default=str(uuid.uuid4()).upper())
    parser.add_argument("--help", action="store_true")
    parser.add_argument("tests", nargs="*")
    args = parser.parse_args()

    if args.help:
        print("Usage: %s" % (USAGE % {"prog": sys.argv[0]}))
        sys.exit(0)

    return args


def load_tests(tests, source):
    if tests:
        return tests

    print("Sourcing tests from %s" % source, flush=True)
    with open(source) as tests_file:
        return [line.rstrip("\n") for line in tests_file]


def git_output(*args):
    return subprocess.check_output(("git",) + args, text=True).strip()


def branch_and_commit():
    try:
        branch = git_output("symbolic-ref", "--quiet", "--short", "HEAD")
    except subprocess.CalledProcessError:
        branch = git_output("rev-parse", "--short", "HEAD")
    commit = git_output("rev-parse", "--short", "HEAD")
    return branch, commit


def dirty_string():
    dirty = []
    status_output = git_output("status", "--porcelain")
    for line in status_output.splitlines():
        match = re.match(r"^ M (.*)", line)
        if match:
            dirty.append(match.group(1))
    return ";".join(dirty)


def print_header(args):
    print("Timeout %s" % args.time)
    print("BatchSize %s" % args.batch)
    print("Runs %s" % args.runs)
    print("Debugger %s" % args.debugger)
    print("Core dir %s" % args.coredir)
    print("Jobid %s" % args.jobid)
    sys.stdout.flush()


def run(default_timeout, test_source, harness_time_option):
    args = parse_args(default_timeout)
    print_header(args)
    tests = load_tests(args.tests, test_source)
    branch, commit = branch_and_commit()
    dirty = dirty_string()

    command = [
        "./harness.py",
        "%s=%s" % (harness_time_option, args.time),
        "--runs=%s" % args.runs,
        "--batch=%s" % args.batch,
        "--each",
        "--debugger=%s" % args.debugger,
        args.coredir,
        args.jobid,
        branch,
        commit,
        dirty,
    ] + tests

    status = subprocess.call(command)
    print("done")
    return status
