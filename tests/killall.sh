#!/bin/bash

# killall.sh
# Kill all sudoku solvers and test scripts that may be running

pkill "test"
pkill "stack-solver"
pkill "sudoku"
pkill "ss-opt"
ps -u $whoami
