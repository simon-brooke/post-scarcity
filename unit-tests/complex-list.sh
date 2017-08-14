#!/bin/bash

expected='(1 2 3 ("Fred") nil 77354)'
actual=`echo '(1 2 3 ("Fred") () 77354)' | target/psse 2> /dev/null`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
    exit 0
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi
