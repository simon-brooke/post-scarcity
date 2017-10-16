#!/bin/bash

expected='5'
actual=`echo "(cond ((equal 2 2) 5))" | target/psse 2> /dev/null | head -2 | tail -1`

if [ "${expected}" = "${actual}" ]
then
  echo "OK"
else
  echo "Fail: expected '${expected}', got '${actual}'"
  exit 1
fi

expected='"should"'
actual=`echo "(cond ((equal 2 3) \"shouldn't\")(t \"should\"))" | target/psse 2> /dev/null | head -2 | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
    exit 0
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi
