#!/bin/bash
TEST=$1
TEST_PID=$2

$(xcrun --find llvm-profdata) merge --sparse "$TEST.$TEST_PID.profraw" -o "$TEST.$TEST_PID.profdata"
$(xcrun --find llvm-cov) report "$TEST" -instr-profile="$TEST.$TEST_PID.profdata"
