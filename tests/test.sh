#!/bin/bash

total=0
pass=0

for file in $2/*
do
    ((total++))
    if $1 $file &> /dev/null; then
        ((pass++))
    else
        echo "Test failed: $file"
    fi
done

echo "==== SUMMARY ===="
echo "Tests passed: $pass/$total"
