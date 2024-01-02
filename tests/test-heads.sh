#!/bin/bash

# Run 10 tests from each level and print results
# Call from parent directory
# Precondition: Directories named according to head${level} with 10 puzzles each

# Usage: test-heads.sh <solver>

heads=( 1_2 1_5 1_7
	2_0 2_3 2_5 2_6 2_8
	3_0 3_2 3_4 3_6 3_8
	4_0 4_1 4_2 4_3 4_4 4_5 4_6 4_7 4_8
	5_0 5_2 5_4 5_5 5_6 5_7 5_8 5_9
	6_0 6_1 6_2 6_3 6_4 6_6 6_7 6_8 6_9
	7_0 7_1 7_2 7_3 7_4 7_5 7_7 7_8 7_9
	8_0 8_1 8_2 8_3 8_4 8_5 8_6 8_7 8_8 8_9
	9_0 9_1 9_2 9_3 ai)

if [[ $# < 1 || ! -x $1 ]] ; then
    echo "Usage: test-heads.sh <solver>"
    exit 1
fi

for head in "${heads[@]}"
do
    echo "--> ${head}"
    tests/test.sh -v $1 "tests/head${head}"
done
