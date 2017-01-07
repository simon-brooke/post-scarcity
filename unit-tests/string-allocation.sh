#!/bin/bash

value='"Fred"'
expected="String cell: character 'F'"
echo ${value} | target/psse 2>&1 | grep "${expected}" > /dev/null

if [ $? -eq 0 ]
then
    echo "OK"
    exit 0
else
    echo "Expected '${expected}', not found"
    exit 1
fi
