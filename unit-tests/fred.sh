#!/bin/bash

expected=\"Fred\"
actual=`echo '"Fred"' | target/psse 2> /dev/null`

if [ "$expected" = "$actual" ]
then
    echo "OK"
    exit 0
else
    echo "Expected '$expected', got '$actual'"
    exit 1
fi
