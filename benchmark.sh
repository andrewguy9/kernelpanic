#!/bin/bash

b1=$1
b2=$2
#TODO can we get these from options
runs=20
batch=1
shift
shift
tests=$@

#TODO can we put job id into db, so that we can limit results to just this run.

dirty=$(git status --porcelain | perl -ne '/^ M (.*)/&& print "$1, "')
if [ $dirty ]
then
  echo "Workspace not clean"
  exit 1
fi

make clean

git checkout $b1

make pc pc_fre

./perf.pl --batch $batch --runs $runs $tests

make clean

git checkout $b2

make pc pc_fre

./perf.pl --batch $batch --runs $runs $tests

#TODO we should not query on branch, use hash instead.
read -d '' sql <<- _EOM_
	create table perf(branch text, commit_hash text, dirty text, test text, readstr text, real_time float, usertext text, user_time float, systext text, sys_time float);
	.separator ","
	.import perf.csv perf
	select b1.test,b2.rt-b1.rt, cast(100*(b2.rt-b1.rt)/b1.rt as int) from (select test,avg(real_time) as rt from perf where branch='${b1}' group by branch,test) as b1 inner join (select test,avg(real_time) as rt from perf where branch='${b2}' group by branch,test) as b2 on b1.test = b2.test;
_EOM_

echo "$sql" | sqlite3

