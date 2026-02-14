#!/bin/bash

result=0

echo -n "$0: plus with fifteen arguments... "

expected="120"
actual=`echo "(+ 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15)" | target/psse 2>/dev/null | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi

echo -n "$0: check that all the args are actually being evaluated... "
expected="120"
actual=`echo "(+ (+ 0 1) (+ 0 2) (+ 0 3) (+ 0 4) (+ 0 5) (+ 0 6) (+ 0 7) (+ 0 8) (+ 0 9) (+ 0 10) (+ 0 11) (+ 0 12) (+ 0 13) (+ 0 14 ) (+ 0 15))" | target/psse 2>/dev/null | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi

exit ${result}
