#!/bin/bash

# Run 10 tests from each level and print results
# Call from parent directory

# Usage: test-heads.sh <solver>

heads=( 1_2 1_5 1_7 2_0 2_3 2_5 2_6 2_8 3_0 3_2 3_4 3_6 3_8 )

for head in "${heads[@]}"
do
    echo "--> ${head}"
    tests/test.sh -v $1 "tests/head${head}"
done
