#!/bin/bash

result=0

expected='5'
actual=`echo "(progn (add 2 3))" | target/psse | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=1
fi

expected='"foo"'
actual=`echo "(progn (add 2.5 3) \"foo\")" | target/psse | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=1
fi

exit ${result}
