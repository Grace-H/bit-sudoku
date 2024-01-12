#!/bin/bash

# bktrk-bench.sh
# Print table of backtrack counts for each puzzle in AI set using various solvers
# Usage: bktrk-bench.sh    # Call from parent of tests directory

printf "%20s %10s %10s %10s %10s\n" "File" "bt" "bt-opt" "ss" "ss-opt"

for f in tests/ai/*
do
	bt=$(./bt ${f})
	btopt=$(./bt-opt ${f})
	ss=$(./ss ${f})
	ssopt=$(./ss-opt ${f})

	printf "%20s %10d %10d %10d %10d\n" ${f#tests/ai/ai-} ${bt} ${btopt} ${ss} ${ssopt}
done

