#!/bin/bash

#####################################################################
# Create a path from root using compact path notation
expected='(-> oblist :users :simon :functions (quote assoc))'
actual=`echo "'/:users:simon:functions/assoc" | target/psse | tail -1`

echo -n "Path from root (oblist) using compact notation: "
if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi

#####################################################################
# Create a path from the current session using compact path notation
expected='(-> session :input-stream)'
actual=`echo "'$:input-stream" | target/psse | tail -1`

echo -n "Path from current session using compact notation: "
if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi


