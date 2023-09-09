#!/bin/bash

total=0
pass=0
verbose=false

while getopts "hv" OPT; do
    case $OPT in
	h)
	    echo "Usage: test.sh [-hv] <solver> <test-dir>"
	    exit 1
	    ;;
	v)
	    verbose=true
	    ;;
    esac
done
shift $((OPTIND - 1))

for file in $2/*
do
    ((total++))
    if $1 $file &> /dev/null ; then
        ((pass++))
    elif [ $verbose = true ] ; then
        echo "Test failed: $file"
    fi
done

echo "($(basename $2)) Tests Passed: $pass/$total"
