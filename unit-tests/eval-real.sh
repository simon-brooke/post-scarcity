#!/bin/bash

# for this test, trailing zeros can be ignored
expected='5.05'
actual=`echo "(eval 5.05)" |\
  target/psse 2> /dev/null |\
  sed 's/0*$//' |\
  head -2 |\
  tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi
