#!/bin/bash

expected='1'
actual=`echo "(apply add '(1))"| target/psse 2> /dev/null | head -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
    exit 0
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi
