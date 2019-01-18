#!/bin/bash

expected='5'
actual=`echo "(add 2 3)" | target/psse | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi

expected='5.5'
actual=`echo "(add 2.5 3)" | target/psse | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
    exit 0
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi

expected='1/4'
actual=`echo "(+ 3/14 1/28)" | target/psse | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi

# (+ integer ratio) should be ratio
expected='25/4'
actual=`echo "(+ 6 1/4)" | target/psse | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi

# (+ ratio integer) should be ratio
expected='25/4'
actual=`echo "(+ 1/4 6)" | target/psse | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi

# (+ real ratio) should be real
# for this test, trailing zeros can be ignored
expected='6.25'
actual=`echo "(+ 6.000000001 1/4)" |\
  target/psse 2> /dev/null |\
  sed 's/0*$//' |\
  head -2 |\
  tail -1`

outcome=`echo "sqrt((${expected} - ${actual})^2) < 0.0000001" | bc`

if [ "${outcome}" = "1" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi

