#!/bin/bash

function join_by { local IFS="$1"; shift; echo "$*"; }
function first { echo "$1"; }
function rest { shift; echo "$@"; }

perf_tests=( $(cat performance.txt regression.txt | grep dbg) )
first_test=$(first "${perf_tests[@]}")
rest_tests=( $(rest "${perf_tests[@]}") )
tests_str=$(join_by "," "${rest_tests[@]}")

$(xcrun --find llvm-profdata) merge -sparse pc_dbg_kern__test*.out.*.profdata -o combined.profdata
$(xcrun --find llvm-cov) report "$first_test" -object="$tests_str" -instr-profile combined.profdata
