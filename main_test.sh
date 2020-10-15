#!/bin/bash

# main test script to check part-1,2,3 executables by calling individual test scripts.
# Takes loop count as input argument.

count=$1
if [ -z "$1" ]
  then
    count=10
fi

mainTest() {
  x=0
  while [ $x -le $count ]
  do
    printf "Count $x - Start part-1 test script:\n\n "
    ./test_part1.sh

    printf "Count $x - Start part-2 test script:\n\n "
    ./test_part2.sh

    printf "Count $x - Start part-3 test script:\n\n "
    ./test_part3.sh
  	
    x=$(( $x + 1 ))
  done
}

mainTest
