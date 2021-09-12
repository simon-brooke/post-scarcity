#!/bin/bash

expected='11'
actual=`echo "(let ((a . 5)(b . 6)) (+ a b))" | target/psse | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '$expected', got '$actual'"
    exit 1
fi

expected='1'
actual=`echo "(let ((a . 5)(b . 6)) (+ a b) (- b a))" | target/psse | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
    exit 0
else
    echo "Fail: expected '$expected', got '$actual'"
    exit 1
fi
