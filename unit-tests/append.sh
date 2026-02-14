#!/bin/bash

return=0;

echo -n "$0: Append two lists... "

expected='(a b c d e f)'
actual=`echo "(append '(a b c) '(d e f))" | target/psse 2>/dev/null | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    return=`echo "${return} + 1" | bc`
fi

echo -n "$0: Append two strings... "

expected='"hellodere"'
actual=`echo '(append "hello" "dere")' | target/psse 2>/dev/null | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    return=`echo "${return} + 1" | bc`
fi

echo -n "$0: Append keyword to string should error... "

expected='Exception:'
actual=`echo '(append "hello" :dere)' | target/psse 2>/dev/null | sed -r '/^\s*$/d' | awk '{print $1}'`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    return=`echo "${return} + 1" | bc`
fi

exit ${return}