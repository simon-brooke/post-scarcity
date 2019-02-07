#!/bin/bash

tmp=hi.$$
echo "Hello, there." > ${tmp}
expected='"Hello, there.'
actual=`echo "(slurp (open \"${tmp}\"))" | target/psse | tail -2 | head -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
    rm ${tmp}
    exit 0
else
    echo "Fail: expected '$expected', got '$actual'"
    exit 1
fi
