#!/bin/bash

result=0

#####################################################################
# Create an empty map using map notation
expected='(2 3 4)'
actual=`echo "(mapcar (lambda (n) (+ n 1)) '(1 2 3))" | target/psse | tail -1`

echo -n "$0: Mapping interpreted function across list: "
if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=1
fi

#####################################################################
# Create an empty map using make-map
expected='("INTR" "REAL" "RTIO" "KEYW")'
actual=`echo "(mapcar type '(1 1.0 1/2 :one))" | target/psse | tail -1`

echo -n "$0: Mapping primitive function across list: "
if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=1
fi
