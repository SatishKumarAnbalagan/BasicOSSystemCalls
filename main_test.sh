#!/bin/bash

# main test script to check part-1,2,3 executables by calling individual test scripts.
# Takes loop count as input argument.

count=$1
if [ -z "$1" ]
  then
    count=10
fi

mainTest() {

    printf "Start part-1 test script:\n\n "
    ./test_part1.sh $count

    printf "Start part-2 test script:\n\n "
    ./test_part2.sh $count

    printf "Start part-3 test script:\n\n "
    ./test_part3.sh $count
}

make clean
make part-1
make part-2
make part-3

mainTest
