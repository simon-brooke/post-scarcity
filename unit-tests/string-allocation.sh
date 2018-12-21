#!/bin/bash

log=log.$$
value='"Fred"'
expected="String cell: character 'F' (70)"
echo ${value} | target/psse -d > ${log} 2>/dev/null
grep "${expected}" ${log} > /dev/null

if [ $? -eq 0 ]
then
    echo "OK"
    rm ${log}
    exit 0
else
    echo "Expected '${expected}', not found in ${log}"
    exit 1
fi
