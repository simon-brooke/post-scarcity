#!/bin/bash

result=0

expected='6'
actual=`echo "(multiply 2 3)" | target/psse | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=1
fi

expected='7.5'
actual=`echo "(multiply 2.5 3)" | target/psse | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=1
fi

exit ${result}