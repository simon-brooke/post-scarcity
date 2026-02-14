#!/bin/bash

result=0

echo -n "$0: We should be able to cons a single character string onto the front of a string... "
expected='"Test"'
actual=`echo '(cons "T" "est")' | target/psse 2>/dev/null | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi

echo -n "$0: But if the first argument has more than one character, we should get a dotted pair... "
expected='("Test" . "pass")'
actual=`echo '(cons "Test" "pass")' | target/psse 2>&1 | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi

exit ${result}

