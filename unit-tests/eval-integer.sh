#!/bin/bash

expected='5'
actual=`echo "(eval 5)" | target/psse 2> /dev/null | head -2 | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi
