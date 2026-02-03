#!/bin/bash

result=0

expected='11'
actual=`echo "(let ((a . 5)(b . 6)) (+ a b))" | target/psse | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '$expected', got '$actual'"
    result=1
fi

expected='1'
actual=`echo "(let ((a . 5)(b . 6)) (+ a b) (- b a))" | target/psse | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '$expected', got '$actual'"
    result=1
fi

exit ${result}