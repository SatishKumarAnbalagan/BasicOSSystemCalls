#!/bin/bash

count=$1
TEST_EXPECT_FILE="testExpect.out"
TEST_OUTPUT_FILE="testOut.out"

mkfifo part-2.pipe
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
    printf "Output:\n\n"
    cat < $TEST_OUTPUT_FILE
	if cmp  $TEST_EXPECT_FILE $TEST_OUTPUT_FILE;
	then
		printf "\n\nSUCCESS !\n\n"
	else 
        printf "\n\nFAIL !\n\nExpected result:\n\n"
        cat < $TEST_EXPECT_FILE
        printf "\n\nPart -3 test failed. Exit !\n"
        exit 1
	fi
}


unit_test1() {
	echo "$TEST1" >> part-2.pipe | sed -r 's/^> //; $d' > $TEST_OUTPUT_FILE
	printf "$TEST1EXPECT" > $TEST_EXPECT_FILE
	compare_file
}

unit_test2() {
	#echo "$TEST2" >> part-2.pipe | sed -r 's/^> //; $d' > $TEST_OUTPUT_FILE
	echo "$TEST2" >> part-2.pipe
	read >&1 | sed -r 's/^> //; $d' > $TEST_OUTPUT_FILE 
	printf "$TEST2EXPECT" > $TEST_EXPECT_FILE
	compare_file
}

unit_test3() {
	echo "$TEST3" >> part-2.pipe | sed -r 's/^> //; $d' > $TEST_OUTPUT_FILE
	echo "$TEST3EXPECT" > $TEST_EXPECT_FILE
	compare_file
}

unit_test4() {
	echo "$TEST4" >> part-2.pipe | sed -r 's/^> //; $d' > $TEST_OUTPUT_FILE
	echo "$TEST4EXPECT" > $TEST_EXPECT_FILE
	compare_file
}

part2test() {
    x=0
    printf "Part -2 test run - $count times:\n"
	./part-2 < part-2.pipe &
	sleep infinity > part-2.pipe &
    while [ $x -le $count ]
    do 
		#unit_test1
		unit_test2
		#unit_test3
		#unit_test4

		x=$(( $x + 1 ))
    done
	#echo "hello" >> part-2.pipe | sed -r 's/^> //; $d' > $TEST_OUTPUT_FILE

	echo "quit" >> part-2.pipe
	rm part-2.pipe
}

part2test

#remove output files
rm $TEST_OUTPUT_FILE $TEST_EXPECT_FILE
