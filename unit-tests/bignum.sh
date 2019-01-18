#!/bin/bash

expected='1,152,921,504,606,846,976'
# 1,152,921,504,606,846,975 is the largest single cell positive integer;
# consequently 1,152,921,504,606,846,976 is the first two cell positive integer.
actual=`echo '(+ 1,152,921,504,606,846,975 1)' | target/psse -v 68 2>bignum.log | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi
