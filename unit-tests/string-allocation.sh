#!/bin/bash

value='"Fred"'
expected="String cell: character 'F'"
# set! protects "Fred" from the garbage collector.
actual=`echo "(set! x ${value})" | target/psse -d 2>&1 | grep "$expected" | sed 's/ *\(.*\) next.*$/\1/'`

if [ $? -eq 0 ]
then
    echo "OK"
    exit 0
else
    echo "$0 => Fail: expected '${expected}', got '${actual}'"
    exit 1
fi
