#!/bin/bash

# killall.sh
# Kill all sudoku solvers and test scripts that may be running

pkill "test"
pkill "ss"
pkill "ts"
pkill "ss-opt"
ps -u $whoami
