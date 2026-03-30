#!/bin/bash

result=0

echo -n "$0: flat list with 16 elements... "
expected="(0 1 2 3 4 5 6 7 8 9 a b c d e f)"

actual=`echo "(list 0 1 2 3 4 5 6 7 8 9 'a 'b 'c 'd 'e 'f)" |\
    target/psse 2>/dev/null |\
    tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '$expected', got '$actual'"
    result=`echo "${result} + 1" | bc`
fi

echo -n "$0: flat list with 5 elements... "
expected="(0 1 2 3 4)"

actual=`echo "(list 0 1 2 3 4)" | target/psse 2>/dev/null | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '$expected', got '$actual'"
    result=`echo "${result} + 1" | bc`
fi

echo -n "$0: flat list with 8 elements... "
expected="(0 1 2 3 4 5 6 7)"

actual=`echo "(list 0 1 2 3 4 5 6 7)" | target/psse 2>/dev/null | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
    exit 0
else
    echo "Fail: expected '$expected', got '$actual'"
    result=`echo "${result} + 1" | bc`
fi

exit ${result}