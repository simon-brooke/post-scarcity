#!/bin/bash

expected='5'
actual=`echo "(add 2 3)" | target/psse 2> /dev/null | head -2 | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
    exit 0
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi

expected='5.5000'
actual=`echo "(add 2.5 3)" | target/psse 2> /dev/null | head -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
    exit 0
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi

