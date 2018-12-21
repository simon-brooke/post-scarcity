#!/bin/bash

# for this test, trailing zeros can be ignored
expected='5.05'
actual=`echo "(eval 5.05)" |\
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


