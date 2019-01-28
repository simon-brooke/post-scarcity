#!/bin/bash

expected='6'
actual=`echo "(apply '(lambda (x y z) (/ (* y z) x)) '(2 3 4))" | target/psse | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi
