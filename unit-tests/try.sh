#!/bin/bash

result=0

echo -n "$0: if the body of a try errors, the last form in the catch block is returned... "
expected=':foo'
actual=`echo "(try ((+ 2 (/ 1 'a))) (:foo))" | target/psse 2>&1 | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    return=`echo "${return} + 1" | bc`
fi

echo -n "$0: if the body of a try errors, the last form in the catch block is evaluated... "

expected='4'
actual=`echo "(try ((+ 2 (/ 1 'a))) ((+ 2 2)))" | target/psse 2>&1 | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    return=`echo "${return} + 1" | bc`
fi

echo -n "$0: body and catch block can optionally be marked with keywords... "
expected='8'
actual=`echo "(try (:body (+ 2 (/ 1 'a))) (:catch (* 2 2 2)))" | target/psse 2>&1 | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    return=`echo "${return} + 1" | bc`
fi

echo -n "$0: the exception is bound to the symbol \`*exception*\` in the catch environment... "
expected='Exception: ((:location . /) (:payload . "Cannot divide: not a number"))'
actual=`echo "(try (:body (+ 2 (/ 1 'a))) (:catch *exception*))" | target/psse 2>&1 | grep Exception`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    return=`echo "${return} + 1" | bc`
fi

exit ${result}
