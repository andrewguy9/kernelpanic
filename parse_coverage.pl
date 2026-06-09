#!/usr/bin/env python3

import argparse
import re
import subprocess
import sys

from test_tools import results_db


COVERAGE_COLUMNS = 13


def parse_args():
    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument("--job", dest="job")
    return parser.parse_args()


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


def split_stats(line):
    return line.replace("%", "").split()


def parse_coverage_report(lines):
    stats = []
    index = 0

    if index < len(lines):
        index += 1
    if index < len(lines):
        index += 1

    while index < len(lines):
        line = lines[index].strip()
        index += 1
        if not line:
            break
        stats.append(split_stats(line))

    if index < len(lines):
        index += 1

    while index < len(lines):
        line = lines[index].strip()
        index += 1
        if re.match(r"^-+$", line):
            break
        row = split_stats(line)
        if row:
            row[3] = "100.0"
            row[6] = "100.0"
            row[9] = "100.0"
            if len(row) > 12:
                row[12] = "100.0"
            stats.append(row)

    if index < len(lines):
        stats.append(split_stats(lines[index].strip()))

    return stats


def normalize_row(row):
    if len(row) != COVERAGE_COLUMNS:
        raise ValueError(
            f"Expected {COVERAGE_COLUMNS} coverage columns, found {len(row)}: {row}"
        )
    return row


def insert_rows(job, branch, commit, dirty, rows):
    with results_db.connect() as connection:
        connection.executemany(
            """
            INSERT INTO coverage (
              job,
              branch,
              commit_hash,
              dirty,
              filename,
              regions,
              missed_regions,
              region_cover,
              functions,
              missed_functions,
              function_cover,
              lines,
              missed_lines,
              line_cover,
              branches,
              missed_branches,
              branch_cover
            )
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            """,
            ([job, branch, commit, dirty] + normalize_row(row) for row in rows))


def main():
    args = parse_args()
    branch, commit = branch_and_commit()
    dirty = dirty_string()

    stats = parse_coverage_report(sys.stdin.read().splitlines())
    insert_rows(args.job, branch, commit, dirty, stats)


if __name__ == "__main__":
    main()
