#!/bin/bash

expected='1/4'
actual=`echo "(+ 3/14 1/28)" | target/psse | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi
