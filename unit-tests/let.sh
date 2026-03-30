#!/bin/bash

result=0

expected='11'
actual=`echo "(let ((a . 5)(b . 6)) (+ a b))" | target/psse | tail -1`
echo -n "$0: let with two bindings, one form in body... "

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '$expected', got '$actual'"
    result=`echo "${result} + 1" | bc`
fi

expected='1'
actual=`echo "(let ((a . 5)(b . 6)) (+ a b) (- b a))" | target/psse | tail -1`
echo -n "$0: let with two bindings, two forms in body..."

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '$expected', got '$actual'"
    result=`echo "${result} + 1" | bc`
fi

exit ${result}