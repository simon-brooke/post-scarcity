#!/bin/bash

result=0

expected='5'
actual=`echo "(cond ((equal 2 2) 5))" | target/psse | tail -1`

if [ "${expected}" = "${actual}" ]
then
  echo "OK"
else
  echo "Fail: expected '${expected}', got '${actual}'"
  result=1
fi

expected='"should"'
actual=`echo "(cond ((equal 2 3) \"shouldn't\")(t \"should\"))" | target/psse | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
 else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=1
fi

exit ${result}