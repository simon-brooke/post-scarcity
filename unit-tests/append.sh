#!/bin/bash

expected='(a b c d e f)'
actual=`echo "(append '(a b c) '(d e f))" | target/psse | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi

expected='"hellodere"'
actual=`echo '(append "hello" "dere")' | target/psse | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi

