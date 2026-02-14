#!/bin/bash

result=1

echo -n "$0: Apply function to one argument... "
expected='1'
actual=`echo "(apply 'add '(1))"| target/psse 2>/dev/null | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi

echo -n "$0: Apply function to multiple arguments... "
expected='3'
actual=`echo "(apply 'add '(1 2))"| target/psse 2>/dev/null | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi

exit ${result}
