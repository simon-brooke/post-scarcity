#!/bin/bash

# Tests for equality.

result=0

echo -n "$0: integers... "

expected="t"
actual=`echo "(= 5 5)" | target/psse 2>/dev/null | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi

echo -n "$0: different integers... "

expected="nil"
actual=`echo "(= 4 5)" | target/psse 2>/dev/null | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi


echo -n "$0: reals... "

expected="t"
actual=`echo "(= 5.001 5.001)" | target/psse 2>/dev/null | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi


echo -n "$0: different reals... "

expected="nil"
actual=`echo "(= 5.001 5.002)" | target/psse 2>/dev/null | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi

echo -n "$0: ratios... "

expected="t"
actual=`echo "(= 4/5 4/5)" | target/psse 2>/dev/null | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi


echo -n "$0: equivalent ratios... "

expected="t"
actual=`echo "(= 4/5 12/15)" | target/psse 2>/dev/null | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi


echo -n "$0: different ratios... "

expected="nil"
actual=`echo "(= 4/5 5/5)" | target/psse 2>/dev/null | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi

echo -n "$0: atoms... "

expected="t"
actual=`echo "(= 'foo 'foo)" | target/psse 2>/dev/null | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi

echo -n "$0: different atoms... "

expected="nil"
actual=`echo "(= 'foo 'bar)" | target/psse 2>/dev/null | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi

echo -n "$0: keywords... "

expected="t"
actual=`echo "(= :foo :foo)" | target/psse 2>/dev/null | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi

echo -n "$0: different keywords... "

expected="nil"
actual=`echo "(= :foo :bar)" | target/psse 2>/dev/null | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi

echo -n "$0: strings... "

expected="t"
actual=`echo '(= "foo" "foo")' | target/psse 2>/dev/null | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi

echo -n "$0: different strings... "

expected="nil"
actual=`echo '(= "foo" "bar")' | target/psse 2>/dev/null | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi

echo -n "$0: maps... "

expected="t"
actual=`echo '(= {:foo 1 :bar 2} {:bar 2 :foo 1})' | target/psse 2>/dev/null | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi

echo -n "$0: different maps... "

expected="nil"
actual=`echo '(= {:foo 1 :bar 2} {:bar 1 :foo 2})' | target/psse 2>/dev/null | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi

exit ${result}
