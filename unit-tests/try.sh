#!/bin/bash

result=0

expected=':foo'
actual=`echo "(try ((+ 2 (/ 1 'a))) (:foo))" | target/psse | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    return=`echo "${return} + 1" | bc`
fi

expected='4'
actual=`echo "(try ((+ 2 (/ 1 'a))) ((+ 2 2)))" | target/psse | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    return=`echo "${return} + 1" | bc`
fi

expected='8'
actual=`echo "(try (:body (+ 2 (/ 1 'a))) (:catch (* 2 2 2)))" | target/psse | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    return=`echo "${return} + 1" | bc`
fi

expected='Exception: "Cannot divide: not a number"'
actual=`echo "(try (:body (+ 2 (/ 1 'a))) (:catch *exception*))" | target/psse | grep Exception`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    return=`echo "${return} + 1" | bc`
fi

exit ${result}
