#!/bin/bash

value=354
expected="(${value} \"INTR\")"
echo "(set! x $value)(list x (type x))" | target/psse 2>&1 | grep "${expected}" > /dev/null

if [ $? -eq 0 ]
then
    echo "OK"
    exit 0
else
    echo "Expected '${expected}', not found"
    exit 1
fi
