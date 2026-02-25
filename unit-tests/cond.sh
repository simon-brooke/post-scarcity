#!/bin/bash

result=0

echo -n "$0: cond with one clause... "

expected='5'
actual=`echo "(cond ((equal? 2 2) 5))" | target/psse 2>/dev/null | tail -1`

if [ "${expected}" = "${actual}" ]
then
  echo "OK"
else
  echo "Fail: expected '${expected}', got '${actual}'"
  result=`echo "${result} + 1" | bc`
fi

echo -n "$0: cond with two clauses... "

expected='"should"'
actual=`echo "(cond ((equal? 2 3) \"shouldn't\")(t \"should\"))" | target/psse 2>/dev/null | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
 else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi

exit ${result}