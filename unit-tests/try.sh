#!/bin/bash

expected=':foo'
actual=`echo "(try ((+ 2 (/ 1 'a))) (:foo))" | target/psse | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi

expected='4'
actual=`echo "(try ((+ 2 (/ 1 'a))) ((+ 2 2)))" | target/psse | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi

expected='8'
actual=`echo "(try (:body (+ 2 (/ 1 'a))) (:catch (* 2 2 2)))" | target/psse | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi

expected=''
actual=`echo "(try (:body (+ 2 (/ 1 'a))) (:catch *exception*))" | target/psse | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi
