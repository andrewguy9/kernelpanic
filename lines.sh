#!/bin/bash

test=$1
pass=$2

$(xcrun --find llvm-cov) show "$test" -instr-profile "$pass"
$(xcrun --find llvm-cov) report "$test" -instr-profile "$pass"
