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

#TODO use parameter parsing and usage checks.
tests=("$@")
first_test=${tests[0]}
objects=()
for (( i = 1; i < ${#tests[@]}; i++ ))
do
  objects+=("-object=${tests[$i]}")
done
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
  $llvm_cov report "$first_test" "${objects[@]}" -instr-profile combined.profdata
  $llvm_cov show "$first_test" "${objects[@]}" -format html -output-dir coverage --instr-profile ./combined.profdata
fi
