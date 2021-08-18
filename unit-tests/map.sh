#!/bin/bash

#####################################################################
# Create an empty map using map notation
expected='{}'
actual=`echo "$expected" | target/psse | tail -1`

echo -n "Empty map using compact map notation: "
if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi

#####################################################################
# Create an empty map using make-map
expected='{}'
actual=`echo "(hashmap)" | target/psse | tail -1`

echo -n "Empty map using (make-map): "
if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi

#####################################################################
# Create a map using map notation: order of keys in output is not
# significant at this stage, but in the long term should be sorted
# alphanumerically
expected='{:one 1, :two 2, :three 3}'
actual=`echo "{:one 1 :two 2 :three 3}" | target/psse | tail -1`

echo -n "Map using map notation: "
if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi

#####################################################################
# Create a map using make-map: order of keys in output is not
# significant at this stage, but in the long term should be sorted
# alphanumerically
expected='{:one 1, :two 2, :three 3}'
actual=`echo "(hashmap nil nil '((:one . 1)(:two . 2)(:three . 3)))" | target/psse | tail -1`

echo -n "Map using (hashmap): "
if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi

#####################################################################
# Keyword in function position
expected='2'
actual=`echo "(:two {:one 1 :two 2 :three 3})" | target/psse | tail -1`

echo -n "Keyword in function position: "
if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi


#####################################################################
# Map in function position
expected='2'
actual=`echo "({:one 1 :two 2 :three 3} :two)" | target/psse | tail -1`

echo -n "Map in function position: "
if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi
