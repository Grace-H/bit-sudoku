#!/bin/bash

# bench-backtrack.sh
# Print table of backtrack counts for each puzzle in AI set using various solvers
# Usage: bench-backtrack.sh    # Call from parent of tests directory

if [[ "$(basename $PWD)" == "tests" ]] ; then
	echo "Re-run from above tests directory."
	echo "Usage: bench-backtrack.sh"
	exit 1
fi

csv=false
while getopts "hc" OPT ; do
	case $OPT in
		c)
			csv=true
			;;
		h)
			echo "Usage: bench-backtrack.sh"
			exit 1
			;;
	esac
done

testdir="tests/sample/"
if $csv ; then
	printf "%s,%s,%s,%s,%s\n" "File" "bt" "bt-opt" "ss" "ss-opt"
else
	printf "%20s %10s %10s %10s %10s\n" "File" "bt" "bt-opt" "ss" "ss-opt"
fi

for f in ${testdir}*
do
	bt=$(./bt ${f})
	btopt=$(./bt-opt ${f})
	ss=$(./ss ${f})
	ssopt=$(./ss-opt ${f})

	if $csv ; then
		printf "%s,%d,%d,%d,%d\n" ${f#${testdir}} ${bt} ${btopt} ${ss} ${ssopt}
	else
		printf "%20s %10d %10d %10d %10d\n" ${f#${testdir}} ${bt} ${btopt} ${ss} ${ssopt}
	fi
done

