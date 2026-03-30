#!/bin/bash

actual=`echo "" | target/psse  2>&1 | tail -2`

alloc=`echo $actual | sed 's/[[:punct:]]/ /g' | awk '{print $4}'`
dealloc=`echo $actual | sed 's/[[:punct:]]/ /g' | awk '{print $6}'`

if [ "${alloc}" = "${dealloc}" ]
then
    echo "OK"
else
    echo "Fail: expected '${alloc}', got '${dealloc}'"
    exit 1
fi
