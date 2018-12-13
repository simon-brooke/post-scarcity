#!/bin/bash

expected="120"
actual=`echo "(+ 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15)" | target/psse 2> /dev/null | head -2 | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
    exit 0
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi
