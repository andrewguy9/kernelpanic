#!/bin/bash

set -e

USAGE="$0 [-r runs] [-b batch_size] dst_branch [branch ...]"

runs=3
batch=1

#TODO add timeout and burn in time params.
while getopts "r:b:h" opt; do
  case ${opt} in
    h)
      echo "help: $USAGE"
      exit 0
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

starting_branch=$(git rev-parse --abbrev-ref HEAD)

if [ "$#" -lt 1 ]; then
    echo "must specify a branch to compare against (master?)"
    echo "$USAGE"
    exit 2
  elif [ "$#" -lt 2 ]; then
    echo "comparing against $starting_branch"
    branches=("${*}" "$starting_branch")
  else
    branches=("${@}")
fi

echo "runs $runs"
echo "batch $batch"

for branch in "${branches[@]}";
do
  if ! git rev-parse --verify --quiet "$branch" > /dev/null; then
    echo "Invalid branch $branch"
    exit 3
  fi
done

jobid=$(uuidgen)

dirty=$(git status --porcelain | perl -ne '/^ M (.*)/&& print "$1, "')
if [ "$dirty" ]
then
  echo "Workspace not clean"
  exit 1
fi

function first { echo "$1"; }
function rest { shift; echo "$@"; }

function process {
  branch=$1;
  echo "testing $branch"
  echo "checking out"
  git checkout "$branch"
  echo "cleaning"
  make clean > /dev/null
  echo "building"
  make pc pc_fre > /dev/null
  echo "Running stress tests"
  ./stress --batch "$batch" --runs "$runs" --jobid "$jobid" --time 10
  echo "Running performance tests"
  ./perf       --batch "$batch" --runs "$runs" --jobid "$jobid"
  ./compute_coverage.sh "$(cat performance.txt stress.txt)" | ./parse_coverage.pl --job "$jobid" >> coverage.csv
}

dst_branch=$(first ${branches[@]})
process "$dst_branch"

for branch in $(rest ${branches[@]})
do
  process "$branch"
  echo "Performance compared to $dst_branch"
  ./perf_report "$dst_branch" "$branch"
  echo "Coverage report"
  ./coverage_report "$jobid" "$dst_branch" "$branch"
done

echo "Restoring $starting_branch"
git checkout "$starting_branch"
