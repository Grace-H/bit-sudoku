#!/bin/bash

# test.sh
# Solve all puzzles in a given directory and print summary of failures
# Run from parent directory. Uses /tests/tmp/ for log files

# Usage: test.sh [-hv] <solver> <test-dir>

function solve(){
    if ! $1 $2 &> /dev/null ; then
        echo "Test failed: $2" >> $3
    fi
}

verbose=false
JOBS_MAX=1
while getopts "hvP:" OPT; do
    case $OPT in
        h)
            echo "Usage: test.sh [-hv] <solver> <test-dir>"
            exit 1
            ;;
        v)
            verbose=true
            ;;
	P)
	    JOBS_MAX=$OPTARG
	    ;;
    esac
done
shift $((OPTIND - 1))

if [[ $# < 2 || ! -x $1 || ! -d $2 ]] ; then
    echo "Usage: test.sh [-hv] <solver> <test-dir>"
    exit 1
fi

total=0
pass=0
level=$(basename $2)
logfile=tests/tmp/log_$level

touch $logfile

for file in $2/*
do
    ((total++))
    while [[ $(jobs -r | wc -l) -ge $JOBS_MAX ]] ; do
        sleep 0.01
    done
    solve $1 $file $logfile &
done
wait

if [ $verbose = true ] ; then
    cat $logfile
fi

echo "($level) Tests Passed: $(($total - $(wc -l < $logfile)))/$total"

rm $logfile
