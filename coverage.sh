#!/bin/bash
#TODO change parameters to bin, set of profiles.
TEST=$1
shift
PROFILES=${*}

#TODO use xcrun only on darwin. What are we to do on linux.
$(xcrun --find llvm-profdata) merge --sparse $PROFILES -o "$TEST.profdata"
$(xcrun --find llvm-cov) report "$TEST" -instr-profile="$TEST.profdata"
