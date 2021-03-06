#!/bin/bash -e

check_string() {
    if [ "$1" != "$2" ] ; then
	echo -e "Expected '$2'\nGot '$1'"
	exit 1
    fi
}

check_cmd_output() {
    cmd=$1
    output="$2"
    if ! ($cmd | fgrep -q "$output") ; then
	echo "'$output' not found in output of '$cmd'"
	exit 1
    fi
}

for i in $(seq 0 10) ; do
    DATES[$i]=$(date -d "$i days ago" +%Y.%m.%d)
done

rm -f test.data
check_cmd_output ./maki-uchi "You did not do your maki-uchi today"
check_cmd_output ./maki-uchi "You did not do maki-uchi at all"
test ! -f test.data
result=`./maki-uchi 10`
check_string "$result" ""
test -f test.data
date "+%Y.%m.%d 10" | diff - test.data
check_cmd_output ./maki-uchi "You did your maki-uchi today"
echo ${DATES[2]} 10 > test.data
check_cmd_output ./maki-uchi "You did not do your maki-uchi today"
check_cmd_output ./maki-uchi "The last date you did your maki-uchi is ${DATES[2]}"
result=`./maki-uchi 10`
echo -e "${DATES[0]} 10\n${DATES[2]} 10" | diff -q - test.data
echo "${DATES[6]}-${DATES[4]} 10" >> test.data
check_cmd_output ./maki-uchi "The earliest date you did your maki-uchi is ${DATES[6]}"

echo "${DATES[6]}-${DATES[4]} 10" > test.data
check_cmd_output ./maki-uchi "You did not do your maki-uchi today"
check_cmd_output ./maki-uchi "The last date you did your maki-uchi is ${DATES[4]}"
check_cmd_output ./maki-uchi "The earliest date you did your maki-uchi is ${DATES[6]}"

echo -e "${DATES[4]}\n${DATES[6]} 10" > test.data
check_cmd_output ./maki-uchi "You did not do your maki-uchi today"
check_cmd_output ./maki-uchi "The last date you did your maki-uchi is ${DATES[4]}"
check_cmd_output ./maki-uchi "You skipped ${DATES[5]}"
check_cmd_output ./maki-uchi "The earliest date you did your maki-uchi is ${DATES[6]}"

echo -e "${DATES[2]}\n${DATES[6]} 10" > test.data
check_cmd_output ./maki-uchi "You did not do your maki-uchi today"
check_cmd_output ./maki-uchi "The last date you did your maki-uchi is ${DATES[2]}"
check_cmd_output ./maki-uchi "You skipped ${DATES[5]} to ${DATES[3]}"
check_cmd_output ./maki-uchi "The earliest date you did your maki-uchi is ${DATES[6]}"

echo -e "${DATES[2]} 10\n${DATES[4]} 10\n${DATES[6]} 10" > test.data
check_cmd_output ./maki-uchi "You did not do your maki-uchi today"
check_cmd_output ./maki-uchi "The last date you did your maki-uchi is ${DATES[2]}"
check_cmd_output ./maki-uchi "You skipped ${DATES[3]} and ${DATES[5]}"
check_cmd_output ./maki-uchi "The earliest date you did your maki-uchi is ${DATES[6]}"

echo -e "${DATES[1]} 10\n${DATES[4]} 10\n${DATES[7]} 10\n${DATES[9]} 10" > test.data
check_cmd_output ./maki-uchi "You did not do your maki-uchi today"
check_cmd_output ./maki-uchi "The last date you did your maki-uchi is ${DATES[1]}"
check_cmd_output ./maki-uchi "You skipped ${DATES[3]} to ${DATES[2]}, ${DATES[6]} to ${DATES[5]} and ${DATES[8]}"
check_cmd_output ./maki-uchi "The earliest date you did your maki-uchi is ${DATES[9]}"

echo -e "${DATES[0]} 6" > test.data
check_cmd_output ./maki-uchi "You only did 6 maki-uchi today"
check_cmd_output ./maki-uchi "You should do 4 more maki-uchi"

echo -e "${DATES[1]} 6" > test.data
check_cmd_output ./maki-uchi "You did not do your maki-uchi today"
check_cmd_output ./maki-uchi "You only did 6 maki-uchi on ${DATES[1]}"
check_cmd_output ./maki-uchi "The last date you did your maki-uchi is ${DATES[1]}"

echo -e "${DATES[1]} 6\n${DATES[3]} 6" > test.data
check_cmd_output ./maki-uchi "You did not do your maki-uchi today"
check_cmd_output ./maki-uchi "You only did 6 maki-uchi on ${DATES[1]}, ${DATES[3]}"
check_cmd_output ./maki-uchi "The last date you did your maki-uchi is ${DATES[1]}"

> test.data
./maki-uchi 20
check_string "$(cat test.data)" "${DATES[1]}-${DATES[0]} 10"

echo ${DATES[1]} 10 > test.data
./maki-uchi 30
check_string "$(cat test.data)" "${DATES[3]}-${DATES[0]} 10"

echo ${DATES[1]} 10 > test.data
./maki-uchi garbage 2> /dev/null ||:
check_string "$(cat test.data)" "${DATES[1]} 10"

test_file=$(mktemp --tmpdir -u maki-uchiXXXXXX)
cleanup() {
    rm -f $test_file
}

trap cleanup EXIT

./maki-uchi -f $test_file 10
test -e $test_file
check_string "$(cat $test_file)" "${DATES[0]} 10"

cmd="./maki-uchi -f$test_file -p"

echo -e "${DATES[1]} 10\n${DATES[4]} 10\n${DATES[7]} 10\n${DATES[9]} 10" > $test_file
check_cmd_output "$cmd" "last ${DATES[1]}"
check_cmd_output "$cmd" "skipped ${DATES[3]}-${DATES[2]} ${DATES[6]}-${DATES[5]} ${DATES[8]}"
check_cmd_output "$cmd" "earliest ${DATES[9]}"

echo "${DATES[0]} 10" > $test_file
check_cmd_output "$cmd" "last today"

> $test_file
check_cmd_output "$cmd" "last never"

./maki-uchi -f $test_file 10 -o -1
check_string "$(cat $test_file)" "${DATES[1]} 10"
