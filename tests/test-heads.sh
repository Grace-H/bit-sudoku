#!/bin/bash

echo "--> 1.2"
./test.sh $1 head1_2

echo "--> 1.5"
./test.sh $1 head1_5

echo "--> 1.7"
./test.sh $1 head1_7

echo "--> 2.0"
./test.sh $1 head2_0

echo "--> 2.3"
./test.sh $1 head2_3

echo "--> 2.5"
./test.sh $1 head2_5

echo "--> 2.6"
./test.sh $1 head2_6

echo "--> 2.8"
./test.sh $1 head2_8

echo "--> 3.0"
./test.sh $1 head3_0

echo "--> 3.2"
./test.sh $1 head3_2

echo "--> 3.4"
./test.sh $1 head3_4

echo "--> 3.6"
./test.sh $1 head3_6

echo "--> 3.8"
./test.sh $1 head3_8
