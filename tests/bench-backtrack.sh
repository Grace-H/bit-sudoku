#!/bin/bash

# bench-backtrack.sh
# Print table of backtrack counts for each puzzle in AI set using various solvers
# Usage: bench-backtrack.sh    # Call from parent of tests directory

if [[ "$(basename $PWD)" == "tests" ]] ; then
	echo "Re-run from above tests directory."
	echo "Usage: bench-backtrack.sh"
	exit 1
fi

testdir="tests/sample/"
printf "%20s %10s %10s %10s %10s\n" "File" "bt" "bt-opt" "ss" "ss-opt"

for f in ${testdir}*
do
	bt=$(./bt ${f})
	btopt=$(./bt-opt ${f})
	ss=$(./ss ${f})
	ssopt=$(./ss-opt ${f})

	printf "%20s %10d %10d %10d %10d\n" ${f#${testdir}} ${bt} ${btopt} ${ss} ${ssopt}
done

