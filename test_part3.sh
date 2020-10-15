#!/bin/bash

count=$1
if [ -z "$1" ]
  then
    count=10
fi

TEST_EXPECT_FILE="testExpect.out"
TEST_OUTPUT_FILE="testOut.out"
COMP_RESULT=0

read -r -d '' TEST1EXPECT << EOM
program 1
program 2
program 1
program 2
program 1
program 2
program 1
program 2
done
EOM

compare_file() {
    cat < $TEST_EXPECT_FILE
	if cmp  $TEST_EXPECT_FILE $TEST_OUTPUT_FILE;
	then
		printf "\n\nSUCCESS !\n\n"
        COMP_RESULT=1
	else 
		printf "\n\nFAIL !\n\n"
        COMP_RESULT=0
	fi
}


unit_test1() {
	printf "unit test-1\n\n"
	out=$(./part-3)
    printf "$out" > $TEST_OUTPUT_FILE
	printf "$TEST1EXPECT" > $TEST_EXPECT_FILE
	compare_file
}

part3test() {
    x=0
    printf "Part -3 test run - $count times:\n "
    while [ $x -le $count ]
    do 
        printf "\ncount $x: "
        unit_test1
        if [ $COMP_RESULT == 0 ]; then
            echo "Part -3 test failed at $x run"
            COMP_RESULT=0
            break;
        fi

		x=$(( $x + 1 ))
    done
}

part3test

#remove output files
rm $TEST_EXPECT_FILE $TEST_OUTPUT_FILE
