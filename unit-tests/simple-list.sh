#!/bin/bash

expected="(1 2 3)"
actual=`echo "'(1 2 3)" | target/psse | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
    exit 0
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi
