#!/bin/bash

result=0;

echo -n "$0: Add two small integers... "

expected='5'
actual=`echo "(add 2 3)" | target/psse 2>/dev/null | sed -r '/^\s*$/d' | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi

echo -n "$0: Add float to integer... "

expected='5.5'
actual=`echo "(add 2.5 3)" | target/psse 2>/dev/null | sed -r '/^\s*$/d' | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi

echo -n "$0: Add two rationals... "

expected='1/4'
actual=`echo "(+ 3/14 1/28)" | target/psse 2>/dev/null | sed -r '/^\s*$/d' | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi

echo -n "$0: Add an integer to a rational... "

# (+ integer ratio) should be ratio
expected='25/4'
actual=`echo "(+ 6 1/4)" | target/psse  2>/dev/null | sed -r '/^\s*$/d' | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi

echo -n "$0: Add a rational to an integer... "

# (+ ratio integer) should be ratio
expected='25/4'
actual=`echo "(+ 1/4 6)" | target/psse 2>/dev/null | sed -r '/^\s*$/d' | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi

echo -n "$0: Add a real to a rational... "

# (+ real ratio) should be real
# for this test, trailing zeros can be ignored
expected='6.25'
actual=`echo "(+ 6.000000001 1/4)" |\
  target/psse 2> /dev/null |\
  sed -r '/^\s*$/d' |\
  sed 's/0*$//' 

outcome=`echo "sqrt((${expected} - ${actual})^2) < 0.0000001" | bc`

if [ "${outcome}" -eq "1" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi

exit ${result}
