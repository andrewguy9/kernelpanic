#!/bin/bash

set -e

branch=$(git branch -v| awk '{if($1=="*"){print $2","$3}}')

jobid=$1
prog=$2

timestr=$({ time $prog 2> /dev/null ; } 2>&1)
stats=$(echo "$timestr" | perl -ne '/^(\w+)\s+(\d+)m(\d+\.\d+)s$/ && print "" . ($2*60 + $3) .","' | perl -pe 's/(.*),$/$1/')
dirty=$(git status --porcelain | perl -ne '/^ M (.*)/&& print "$1;"' | perl -pe 's/(.*);$/$1/')

echo "$jobid,$branch,$dirty,$prog,$stats" | tee -a ./perf.csv
