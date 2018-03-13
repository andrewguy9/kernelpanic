#!/bin/bash

branch=$(git branch -v| awk '{if($1=="*"){print $2", "$3}}')

jobid=$1
prog=$2

#TODO we don't need the stat name.
stats=$({ time $prog 2> /dev/null ; } 2>&1 | perl -ne '/^(\w+)\s+(\d+)m(\d+\.\d+)s$/ && print "$1,". ($2*60 + $3) .","' | perl -pe 's/(.*),$/$1/')
#TODO We should not generate a report if the program crashed.
dirty=$(git status --porcelain | perl -ne '/^ M (.*)/&& print "$1, "')

echo $jobid,$branch,$dirty,$prog,$stats | tee -a ./perf.csv
