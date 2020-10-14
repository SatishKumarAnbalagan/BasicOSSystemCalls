#!/bin/bash

make clean
make part-2

TEST_EXPECT_FILE="testExpect.out"
TEST_OUTPUT_FILE="testOut.out"
NEW_LINE="\n"

read -r -d '' TEST1 << EOM
wait
EOM

TEST1EXPECT=""


read -r -d '' TEST2 << EOM
hello
EOM

read -r -d '' TEST2EXPECT << EOM
Hello world!
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

read -r -d '' TEST4 << EOM
$TEST3

$TEST2
$TEST2
$TEST3
EOM

read -r -d '' TEST4EXPECT << EOM
$TEST3EXPECT
$TEST2EXPECT
$TEST2EXPECT
$TEST3EXPECT
EOM


compare_file() {
	if cmp  $TEST_EXPECT_FILE $TEST_OUTPUT_FILE;
	then
		printf "success!\n\n"
	else 
		printf "fail!\n\n"
	fi
}


unit_test1() {
	printf "Test1 StdInput:\n "
	echo "$TEST1"
	out=`./part-2 << END_TEXT
$TEST1
quit
END_TEXT`
    echo "expected output:"
	echo "$TEST1EXPECT"
	echo "$out"  | sed -r 's/^> //; $d' > $TEST_OUTPUT_FILE
	printf "$TEST1EXPECT" > $TEST_EXPECT_FILE
	compare_file
}

unit_test1 

unit_test2() {
	printf "Test2 StdInput:\n "
	echo "$TEST2"
	out=`./part-2 << END_TEXT
$TEST2
quit
END_TEXT`
    echo "expected output:"
	echo "$TEST2EXPECT"
	echo "$out"  | sed -r 's/^> //; $d' > $TEST_OUTPUT_FILE
	echo "$TEST2EXPECT" > $TEST_EXPECT_FILE
	compare_file
}

unit_test2

unit_test3() {
	printf "Test3 StdInput:\n "
	echo "$TEST3"
	out=`./part-2 << END_TEXT
$TEST3

quit
END_TEXT`
    echo "expected output:"
	echo "$TEST3EXPECT"
	echo "$out"  | sed -r 's/^> //; $d' > $TEST_OUTPUT_FILE
	echo "$TEST3EXPECT" > $TEST_EXPECT_FILE
	compare_file
}

unit_test3

unit_test4() {
	printf "Test4 StdInput:\n "
	echo "$TEST4"
	out=`./part-2 << END_TEXT
$TEST4

quit
END_TEXT`
    echo "expected output:"
	echo "$TEST4EXPECT"
	echo "$out"  | sed -r 's/^> //; $d' > $TEST_OUTPUT_FILE
	echo "$TEST4EXPECT" > $TEST_EXPECT_FILE
	compare_file
}

unit_test4

#remove output files
rm $TEST_OUTPUT_FILE $TEST_EXPECT_FILE

