#!/bin/bash

# Run all tests from each level and print results
# Call from parent directory

# Usage: test-all.sh <solver>

tests=( 1_2 1_5 1_7 2_0 2_3 2_5 2_6 2_8 3_0 3_2 3_4 3_6 3_8 )

for t in "${tests[@]}"
do
    tests/test.sh $1 "tests/se${t}" &
done
