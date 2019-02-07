#!/bin/bash

expected='"λάμ(β)δα"'
actual=`echo $expected | target/psse | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi
