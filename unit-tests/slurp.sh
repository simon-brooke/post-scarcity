#!/bin/bash

expected='"Hello, this is used by `slurp.sh` test, please do not remove.'
actual=`echo '(slurp (open "hi"))' | target/psse | tail -2 | head -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
    exit 0
else
    echo "Fail: expected '$expected', got '$actual'"
    exit 1
fi
