#!/bin/bash

result=0

#####################################################################
# Create a path from root using compact path notation
echo -n "$0: Create a path from root using compact path notation... "
expected='(-> oblist :users :simon :functions (quote assoc))'
actual=`echo "'/:users:simon:functions/assoc" | target/psse 2>&1 | tail -1`

echo -n "Path from root (oblist) using compact notation: "
if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi

#####################################################################
# Create a path from the current session using compact path notation
echo -n "$0: Create a path from the current session using compact path notation... "
expected='(-> session :input-stream)'
actual=`echo "'$:input-stream" | target/psse 2>/dev/null | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi

exit ${result}

