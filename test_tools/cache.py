import argparse
import sys

from test_tools import results_db


def parse_args():
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest="command", required=True)

    find_parser = subparsers.add_parser("find")
    find_parser.add_argument("--runs", type=int, required=True)
    find_parser.add_argument("--coverage-required", action="store_true")
    find_parser.add_argument("commit")
    find_parser.add_argument("tests", nargs="+")

    copy_parser = subparsers.add_parser("copy")
    copy_parser.add_argument("--coverage-required", action="store_true")
    copy_parser.add_argument("source_job")
    copy_parser.add_argument("target_job")
    copy_parser.add_argument("commit")

    return parser.parse_args()


def coverage_jobs(commit):
    with results_db.connect() as connection:
        cursor = connection.execute(
            """
            SELECT DISTINCT job
            FROM coverage
            WHERE commit_hash = ? AND filename = 'TOTAL'
            """,
            (commit,))
        return {row[0] for row in cursor}


def find_perf_job(commit, runs, tests, coverage_required):
    expected_tests = list(dict.fromkeys(tests))
    placeholders = ",".join("?" for _ in expected_tests)
    coverage_filter = coverage_jobs(commit) if coverage_required else None
    if coverage_required and not coverage_filter:
        return None

    with results_db.connect() as connection:
        cursor = connection.execute(
            """
            SELECT
              job,
              max(rowid) as newest_row
            FROM perf candidate
            WHERE
              commit_hash = ?
              AND dirty = ''
              AND NOT EXISTS (
                SELECT 1
                FROM perf failure
                WHERE
                  failure.job = candidate.job
                  AND failure.commit_hash = candidate.commit_hash
                  AND failure.result != 'SUCCESS'
              )
              AND test IN (%s)
            GROUP BY job
            HAVING COUNT(DISTINCT test) = ?
              AND MIN((
                SELECT COUNT(*)
                FROM perf repeated
                WHERE
                  repeated.job = candidate.job
                  AND repeated.commit_hash = candidate.commit_hash
                  AND repeated.test = candidate.test
                  AND repeated.result = 'SUCCESS'
              )) >= ?
            ORDER BY newest_row DESC
            """ % placeholders,
            (commit,) + tuple(expected_tests) + (len(expected_tests), runs))
        for row in cursor:
            job = row[0]
            if coverage_filter is None or job in coverage_filter:
                return job
    return None


def copy_perf_rows(source_job, target_job, commit):
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
            SELECT
              ?,
              branch,
              commit_hash,
              dirty,
              test,
              real,
              user,
              sys,
              status,
              result
            FROM perf
            WHERE job = ? AND commit_hash = ?
            """,
            (target_job, source_job, commit))


def copy_coverage_rows(source_job, target_job, commit):
    with results_db.connect() as connection:
        cursor = connection.execute(
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
            SELECT
              ?,
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
            FROM coverage
            WHERE job = ? AND commit_hash = ?
            """,
            (target_job, source_job, commit))
        return cursor.rowcount > 0


def main():
    args = parse_args()
    if args.command == "find":
        job = find_perf_job(
            args.commit,
            args.runs,
            args.tests,
            args.coverage_required)
        if job is None:
            return 1
        print(job)
        return 0

    if args.command == "copy":
        copy_perf_rows(args.source_job, args.target_job, args.commit)
        if args.coverage_required and not copy_coverage_rows(
                args.source_job, args.target_job, args.commit):
            print(
                "failed to copy cached coverage rows for %s" % args.commit,
                file=sys.stderr)
            return 1
        if not args.coverage_required:
            copy_coverage_rows(args.source_job, args.target_job, args.commit)
        return 0

    raise RuntimeError("unknown command %s" % args.command)


if __name__ == "__main__":
    sys.exit(main())
