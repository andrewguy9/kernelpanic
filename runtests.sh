#!/bin/bash

set -e

USAGE="$0 [-f] [-r runs] [-b batch_size] [feature_rev [baseline_rev]]"

runs=3
batch=1
force=0

#TODO add timeout and burn in time params.
while getopts "fr:b:h" opt; do
  case ${opt} in
    h)
      echo "help: $USAGE"
      exit 0
      ;;
    f)
      force=1
      ;;
    r)
      runs=$OPTARG
      ;;
    b)
      batch=$OPTARG
      ;;
    [?])
      echo "Usage: $USAGE"
      exit 1
      ;;
    *)
      echo "Usage: $USAGE"
      exit 1
      ;;
  esac
done
shift $((OPTIND -1))

function require_command {
  local command_name=$1
  if ! command -v "$command_name" > /dev/null; then
    echo "Missing required command: $command_name"
    exit 4
  fi
}

function require_positive_integer {
  name=$1
  value=$2
  if ! [[ "$value" =~ ^[1-9][0-9]*$ ]]; then
    echo "$name must be a positive integer: $value"
    exit 2
  fi
}

require_positive_integer "-r runs" "$runs"
require_positive_integer "-b batch_size" "$batch"
require_command git
require_command python3
require_command sqlite3
require_command uuidgen

starting_checkout=$(git symbolic-ref --quiet --short HEAD || git rev-parse --verify HEAD)
restore_needed=0

function restore_checkout {
  if [ "$restore_needed" -eq 1 ]; then
    echo "Restoring $starting_checkout"
    git checkout "$starting_checkout"
    restore_needed=0
  fi
}

trap restore_checkout EXIT

if [ "$#" -gt 2 ]; then
    echo "Wrong number of args $#, expected zero, one, or two revs"
    echo "$USAGE"
    exit 2
fi

echo "runs $runs"
echo "batch $batch"

revs=("$@")
if [ "${#revs[@]}" -eq 0 ]; then
  revs=("$starting_checkout")
fi

short_commits=()
checkout_commits=()
for rev in "${revs[@]}"; do
  if ! checkout_commit=$(git rev-parse --verify "$rev^{commit}" 2> /dev/null); then
    echo "Invalid rev $rev"
    exit 3
  fi
  short_commit=$(git rev-parse --short "$checkout_commit")
  checkout_commits+=("$checkout_commit")
  short_commits+=("$short_commit")
done

jobid=$(uuidgen)

dirty=()
while IFS= read -r line
do
  if [[ $line =~ ^\ M\ (.*)$ ]]; then
    dirty+=("${BASH_REMATCH[1]}")
  fi
done < <(git status --porcelain)

if [ "${#dirty[@]}" -ne 0 ]
then
  echo "Workspace not clean"
  exit 1
fi

coverage_tests=()
function require_readable_test_list {
  source_file=$1
  if [ ! -r "$source_file" ]; then
    echo "Cannot read test list: $source_file"
    exit 4
  fi
}

require_readable_test_list performance.txt
require_readable_test_list stress.txt

while IFS= read -r test
do
  if [ -n "$test" ]; then
    coverage_tests+=("$test")
  fi
done < performance.txt

while IFS= read -r test
do
  if [ -n "$test" ]; then
    coverage_tests+=("$test")
  fi
done < stress.txt

function require_test_executables {
  missing=()
  for test in "$@"
  do
    if [ ! -x "$test" ]; then
      missing+=("$test")
    fi
  done

  if [ "${#missing[@]}" -ne 0 ]; then
    echo "Missing test executable(s):"
    printf '  %s\n' "${missing[@]}"
    exit 4
  fi
}

function require_debugger {
  if ! command -v gdb > /dev/null && ! command -v lldb > /dev/null; then
    echo "Missing required debugger: install gdb or lldb"
    exit 4
  fi
}

function require_coverage_tools {
  if [[ "$OSTYPE" == "darwin"* ]]; then
    require_command xcrun
    if ! xcrun --find llvm-profdata > /dev/null; then
      echo "Missing required coverage tool: llvm-profdata"
      exit 4
    fi
    if ! xcrun --find llvm-cov > /dev/null; then
      echo "Missing required coverage tool: llvm-cov"
      exit 4
    fi
  else
    require_command llvm-profdata
    require_command llvm-cov
  fi
}

coverage_required=0
if [[ "$OSTYPE" == "darwin"* ]]; then
  coverage_required=1
fi

function cache_args {
  if [ "$coverage_required" -eq 1 ]; then
    echo "--coverage-required"
  fi
}

function reuse_cached_results {
  short_commit=$1
  if [ "$force" -eq 1 ]; then
    return 1
  fi

  if cached_job=$(python3 -m test_tools.cache find \
      --runs "$runs" \
      $(cache_args) \
      "$short_commit" \
      "${coverage_tests[@]}"); then
    echo "reusing cached job $cached_job for $short_commit"
    python3 -m test_tools.cache copy \
      $(cache_args) \
      "$cached_job" \
      "$jobid" \
      "$short_commit"
    return 0
  fi

  return 1
}

function process_current {
  require_command make
  require_debugger
  if [[ "$OSTYPE" == "darwin"* ]]; then
    require_coverage_tools
  fi
  echo "cleaning"
  make clean > /dev/null
  echo "building"
  make pc pc_fre > /dev/null
  require_test_executables "${coverage_tests[@]}"
  echo "Running stress tests"
  ./stress --batch "$batch" --runs "$runs" --jobid "$jobid" --time 10
  echo "Running performance tests"
  ./perf       --batch "$batch" --runs "$runs" --jobid "$jobid"
  if [[ "$OSTYPE" == "darwin"* ]]; then
    ./compute_coverage.sh "${coverage_tests[@]}" | ./parse_coverage.pl --job "$jobid"
  fi
}

function process_rev {
  rev=$1
  checkout_commit=$2
  short_commit=$3
  checkout=$4

  echo "testing $rev ($short_commit)"
  if reuse_cached_results "$short_commit"; then
    return
  fi

  if [ "$checkout" -eq 1 ]; then
    echo "checking out"
    git checkout "$checkout_commit"
    restore_needed=1
  fi
  process_current
}

if [ "$#" -eq 0 ]; then
  process_rev "$starting_checkout" "${checkout_commits[0]}" "${short_commits[0]}" 0
  echo "Performance report"
  ./perf_report --job "$jobid" "${short_commits[0]}"
  echo "Coverage report"
  ./coverage_report "$jobid" "${short_commits[0]}"
elif [ "$#" -eq 1 ]; then
  process_rev "${revs[0]}" "${checkout_commits[0]}" "${short_commits[0]}" 1
  restore_checkout
  echo "Performance report"
  ./perf_report --job "$jobid" "${short_commits[0]}"
  echo "Coverage report"
  ./coverage_report "$jobid" "${short_commits[0]}"
else
  feature_rev=${revs[0]}
  baseline_rev=${revs[1]}
  feature_checkout_commit=${checkout_commits[0]}
  baseline_checkout_commit=${checkout_commits[1]}
  feature_commit=${short_commits[0]}
  baseline_commit=${short_commits[1]}

  process_rev "$feature_rev" "$feature_checkout_commit" "$feature_commit" 1
  process_rev "$baseline_rev" "$baseline_checkout_commit" "$baseline_commit" 1
  restore_checkout
  echo "Performance compared to $baseline_rev ($baseline_commit)"
  ./perf_report --job "$jobid" "$baseline_commit" "$feature_commit"
  echo "Coverage report"
  ./coverage_report "$jobid" "$baseline_commit" "$feature_commit"
fi
