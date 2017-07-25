#!/bin/sh -e

check_string() {
    if [ "$1" != "$2" ] ; then
	/bin/echo -e "Expected '$2'\nGot '$1'"
	exit 1
    fi
}

rm -f test.data
check_string "`./maki-uchi`" "You did not do your maki-uchi today"
test ! -f test.data
result=`./maki-uchi 10`
check_string "$result" ""
test -f test.data
date +%Y.%m.%d | diff -q - test.data
check_string "`./maki-uchi`" "You did your maki-uchi today"
date -d "2 days ago" +%Y.%m.%d > test.data
check_string "`./maki-uchi`" "You did not do your maki-uchi today"
