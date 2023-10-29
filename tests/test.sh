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

if [[ $# < 2 ]] || ! [[ -d $2 ]] ; then
    echo "Usage: test.sh [-hv] <solver> <test-dir>"
    exit 1
fi

total=0
pass=0
level=$(basename $2)
logfile=tests/tmp/log_$level

touch $logfile

job_count=0
JOBS_MAX=48
for file in $2/*
do
    ((total++))
    ((jobs_count++))
    if [ $jobs_count > $JOBS_MAX ] ; then
	wait -n
    fi
    solve $1 $file $logfile &
    ((job_count--))
done
wait

if [ $verbose = true ] ; then
    cat $logfile
fi

echo "($level) Tests Passed: $(($total - $(wc -l < $logfile)))/$total"

rm $logfile
