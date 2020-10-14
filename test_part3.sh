#!/bin/bash

make clean
make part-3

TEST_EXPECT_FILE="testExpect.out"
TEST_OUTPUT_FILE="testOut.out"



read -r -d '' TEST1 << EOM
hello
EOM

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


read -r -d '' TEST3 << EOM
ugrep abc
abcd
cba
EOM

read -r -d '' TEST3EXPECT << EOM
ugrep: enter blank line to quit
-- abcd
EOM

COMP_RESULT=0

compare_file() {
	if cmp  $TEST_EXPECT_FILE $TEST_OUTPUT_FILE;
	then
		printf "success!\n\n"
        COMP_RESULT=1
	else 
		printf "fail!\n\n"
	fi
}


unit_test1() {
	printf "Test1 Run program 1 time:\n "
	out=$(./part-3)
    echo "expected output:"
	echo "$TEST1EXPECT"
	printf "$out" > $TEST_OUTPUT_FILE
	printf "$TEST1EXPECT" > $TEST_EXPECT_FILE
	compare_file
}

unit_test1 

unit_test2() {
    end=10;
	printf "Test2 Run program $end times:\n "
    for i in $(seq 1 $end);
        do echo "Run number: $i";
    	out=$(./part-3)
        echo "expected output:"
        echo "$TEST1EXPECT"
        printf "$out" > $TEST_OUTPUT_FILE
        printf "$TEST1EXPECT" > $TEST_EXPECT_FILE
        compare_file
        if [ $COMP_RESULT == 0 ]; then
            echo "Test failed at Run number $i"
            COMP_RESULT=0
            break;
        fi

    done;
}

unit_test2


#remove output files
rm $TEST_EXPECT_FILE $TEST_OUTPUT_FILE

