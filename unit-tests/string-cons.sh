#!/bin/bash

# We should be able to cons a single character string onto the front of a string
expected='"Test"'
actual=`echo '(cons "T" "est")' | target/psse | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi

# But if the first argument has more than one character, we should get a dotted pair
expected='("Test" . "pass")'
actual=`echo '(cons "Test" "pass")' | target/psse | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi
