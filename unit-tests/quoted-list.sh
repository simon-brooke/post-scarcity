#!/bin/bash

expected='(123 (4 (5 nil)) Fred)'
actual=`echo "'(123 (4 (5 ())) Fred)" | target/psse 2> /dev/null | head -2 | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
    exit 0
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi
