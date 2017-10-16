#!/bin/bash

expected='5'
actual=`echo "(progn '((add 2 3)))" | target/psse 2> /dev/null | head -2 | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
    exit 0
else
    echo "Fail: expected '${expected}', got '${actual}'"
fi

expected='"foo"'
actual=`echo "(progn '((add 2.5 3) \"foo\"))" | target/psse 2> /dev/null | head -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
    exit 0
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi
