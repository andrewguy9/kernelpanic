#!/bin/bash

#TODO rename this script.
#TODO change parameters to a bin, and a set of coverages.
test=$1
pass=$2

#TODO use xcrun only on Darwin.
$(xcrun --find llvm-cov) show "$test" -instr-profile "$pass"
$(xcrun --find llvm-cov) report "$test" -instr-profile "$pass"
