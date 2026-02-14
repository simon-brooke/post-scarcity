#!/bin/bash

result=0

echo -n "$0: progn with one form... "
expected='5'
actual=`echo "(progn (add 2 3))" | target/psse 2>/dev/null | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi

echo -n "$0: progn with two forms... "
expected='"foo"'
actual=`echo "(progn (add 2.5 3) \"foo\")" | target/psse 2>/dev/null | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi

exit ${result}
