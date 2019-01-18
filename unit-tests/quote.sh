#!/bin/bash

expected='Fred'
actual=`echo "'Fred" | target/psse | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
    exit 0
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi
