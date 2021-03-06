#!/bin/bash

if [[ "$OSTYPE" == "darwin"* ]]; then
  if ! profdata=$(xcrun --find llvm-profdata);
  then
    >&2 echo "Cannot find llvm-profdata"
    exit 1
  fi
  if ! llvm_cov=$(xcrun --find llvm-cov);
  then
    >&2 echo "Cannot find llvm-cov"
    exit 1
  fi
else
  if ! profdata=$(which llvm-profdata);
  then
    >&2 echo "Cannot find llvm-profdata"
    exit 1
  fi
  if ! llvm_cov=$(which llvm-cov);
  then
    >&2 echo "Cannot find llvm-cov"
    exit 1
  fi
fi

function join_by { local IFS="$1"; shift; echo "$*"; }
function first { echo "$1"; }
function rest { shift; echo "$@"; }

#TODO use parameter parsing and usage checks.
tests=${*}
first_test=$(first ${tests[@]})
rest_tests=$(rest ${tests[@]})
tests_str=$(join_by "," ${rest_tests[@]})
datas=()

shopt -s nullglob
for test in $tests
do
  for raw in "$test".*.profraw
  do
    data="${raw%.profraw}.profdata"
    $profdata merge --sparse "$raw" -o "$data"
    datas=("${datas[@]}" $data)
  done
done

if [ ${#datas[@]} -ne 0 ]; then
  $profdata merge -sparse "${datas[@]}" -o combined.profdata
  $llvm_cov report "$first_test" -object="$tests_str" -instr-profile combined.profdata
  $llvm_cov show "$first_test" -object="$tests_str" -format html -output-dir coverage --instr-profile ./combined.profdata
fi
