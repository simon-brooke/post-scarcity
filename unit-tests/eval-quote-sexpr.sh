#!/bin/bash

expected='5'
actual=`echo "(eval '(add 2 3))" | target/psse | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi
