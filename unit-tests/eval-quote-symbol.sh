#!/bin/bash

expected='<Special form: ((:primitive . t) (:name . cond))>'
actual=`echo "(eval 'cond)" | target/psse | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi
