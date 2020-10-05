#!/bin/bash

TEST_STDIN_FILE="testStdIn.out"
TEST_STDOUT_FILE="testStdOut.out"
NEW_LINE="\n"

read -r -d '' TEST1 << EOM
This is line 1.
This is line 2.
Line 3.
EOM


read -r -d '' TEST2 << EOM

This is line 2.
>New line 3
EOM


read -r -d '' TEST3 << EOM
line 1.
This is line 2.
df
EOM


compare_file() {
	if cmp  $TEST_STDIN_FILE $TEST_STDOUT_FILE;
	then
		printf "success\n"
	else 
		printf "fail\n"
	fi
}


unit_test1() {
	printf "Test1 StdInput: \n"
	echo "$TEST1"
	out=`./part-1 << END_TEXT
$TEST1
quit
END_TEXT`

	echo "$out"
	echo "$out" | sed -r 's/^> you typed: //; $d' > $TEST_STDOUT_FILE
	echo "$TEST1" > $TEST_STDIN_FILE
	compare_file
}

unit_test1 

unit_test2() {
	printf "Test2 StdInput:\n "
	echo "$TEST2"
	out=`./part-1 << END_TEXT
$TEST2
quit
END_TEXT`

	echo "$out"
	echo "$out" | sed -r 's/^> you typed: //; $d' > $TEST_STDOUT_FILE
	echo "$TEST2" > $TEST_STDIN_FILE
	compare_file
}

unit_test2

unit_test3() {
	printf "Test3 StdInput:\n "
	echo "$TEST3"
	out=`./part-1 << END_TEXT
$TEST3
quit
END_TEXT`

	echo "$out"
	echo "$out" | sed -r 's/^> you typed: //; $d' > $TEST_STDOUT_FILE
	echo "$TEST3" > $TEST_STDIN_FILE
	compare_file
}

unit_test3


#remove output files
rm $TEST_STDIN_FILE $TEST_STDOUT_FILE

