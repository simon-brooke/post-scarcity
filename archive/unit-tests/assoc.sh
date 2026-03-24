#!/bin/bash

result=0

expected='1'
actual=`echo "(assoc 'foo '((foo . 1) (bar . 2) {ban 3 froboz 4 foo 5} (foobar . 6)))" | target/psse | tail -1`


echo -n "$0 $1: assoc list binding... "

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi

expected='4'
actual=`echo "(assoc 'froboz '((foo . 1) (bar . 2) {ban 3 froboz 4 foo 5} (foobar . 6)))" | target/psse | tail -1`


echo -n "$0 $1: hashmap binding... "

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi

expected='nil'
actual=`echo "(assoc 'ban '((foo . 1) (bar . 2) {ban nil froboz 4 foo 5} (foobar . 6) (ban . 7)))" | target/psse | tail -1`


echo -n "$0 $1: key bound to 'nil' (1)... "

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi

expected='nil'
actual=`echo "(assoc 'foo '((foo . nil) (bar . 2) {ban 3 froboz 4 foo 5} (foobar . 6)))" | target/psse | tail -1`


echo -n "$0 $1: key bound to nil (2)... "

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi

