#!/bin/bash
#TODO change parameters to bin, set of profiles.
TEST=$1
TEST_PID=$2

#TODO use xcrun only on darwin. What are we to do on linux.
$(xcrun --find llvm-profdata) merge --sparse "$TEST.$TEST_PID.profraw" -o "$TEST.$TEST_PID.profdata"
$(xcrun --find llvm-cov) report "$TEST" -instr-profile="$TEST.$TEST_PID.profdata"
