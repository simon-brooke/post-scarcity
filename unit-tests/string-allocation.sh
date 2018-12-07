#!/bin/bash

value='"Fred"'
expected="String cell: character 'F' (70)"
echo ${value} | target/psse -d 2>/dev/null | grep "${expected}" > /dev/null

if [ $? -eq 0 ]
then
    echo "OK"
    exit 0
else
    echo "Expected '${expected}', not found"
    exit 1
fi
