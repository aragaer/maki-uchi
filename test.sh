#!/bin/sh -e

check_string() {
    if [ "$1" != "$2" ] ; then
	/bin/echo -e "Expected '$2'\nGot '$1'"
	exit 1
    fi
}

check_cmd_output() {
    cmd=$1
    output="$2"
    if ! ($cmd | grep -q "$output") ; then
	/bin/echo "'$output' not found in output of '$cmd'"
	exit 1
    fi
}

rm -f test.data
check_cmd_output ./maki-uchi "You did not do your maki-uchi today"
check_cmd_output ./maki-uchi "You did not do maki-uchi at all"
test ! -f test.data
result=`./maki-uchi 10`
check_string "$result" ""
test -f test.data
date +%Y.%m.%d | diff -q - test.data
check_cmd_output ./maki-uchi "You did your maki-uchi today"
two_days_ago=$(date -d "2 days ago" +%Y.%m.%d)
echo $two_days_ago > test.data
check_cmd_output ./maki-uchi "You did not do your maki-uchi today"
check_cmd_output ./maki-uchi "The last date you did your maki-uchi is $two_days_ago"
result=`./maki-uchi 10`
today=$(date +%Y.%m.%d)
/bin/echo -e "$today\n$two_days_ago" | diff -q - test.data
check_cmd_output ./maki-uchi "The last date you did your maki-uchi is $two_days_ago"

#TODO:
# Should also print past info
# Properly roll over maki-uchi to the past dates
