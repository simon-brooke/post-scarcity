#!/bin/bash

result=0

echo -n "$0: multiply two integers... "

expected='6'
actual=`echo "(multiply 2 3)" | target/psse 2>/dev/null | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi

echo -n "$0: multiply a real by an integer... "

expected='7.5'
actual=`echo "(multiply 2.5 3)" | target/psse 2>/dev/null | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi

exit ${result}