#!/bin/bash

# Run all tests from each level and print results
# Call from parent directory

# Usage: test-all.sh <solver>

tests=( se1_2 se1_5 se1_7
	se2_0 se2_3 se2_5 se2_6 se2_8
	se3_0 se3_2 se3_4 se3_6 se3_8
	se4_0 se4_1 se4_2 se4_3 se4_4 se4_5 se4_6 se4_7 se4_8
	se5_0 se5_2 se5_4 se5_5 se5_6 se5_7 se5_8 se5_9
	se6_0 se6_1 se6_2 se6_3 se6_4 se6_6 se6_7 se6_8 se6_9
	se7_0 se7_1 se7_2 se7_3 se7_4 se7_5 se7_7 se7_8 se7_9
	se8_0 se8_1 se8_2 se8_3 se8_4 se8_5 se8_6 se8_7 se8_8 se8_9
	se9_0 se9_1 se9_2 se9_3 ai )

if [[ $# < 1 ]] ; then
    echo "Usage: test-all.sh <solver>"
    exit 1
fi

for t in "${tests[@]}"
do
    tests/test.sh $1 "tests/${t}" &
done
wait
