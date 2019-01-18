#!/bin/bash

#####################################################################
# subtract a smallnum from a smallnum to produce a smallnum
# (right on the boundary)
a=1152921504606846976
b=1
expected='1152921504606846975'
output=`echo "(- $a $b)" | target/psse -v 2 2>psse.log`

actual=`echo $output |\
  tail -1 |\
  sed 's/\,//g'`

echo -n "subtracting $b from $a: "
if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi

echo -n "checking no bignum was created: "
grep -v 'BIGNUM!' psse.log > /dev/null
if [ $? -eq "0" ]
then
    echo "OK"
else
    echo "Fail"
    exit 1
fi

#####################################################################
# subtract a smallnum from a bignum to produce a smallnum
# (just over the boundary)
a='1152921504606846977'
b=1
expected='1152921504606846976'
output=`echo "(- $a $b)" | target/psse -v 2 2>psse.log`

actual=`echo $output |\
  tail -1 |\
  sed 's/\,//g'`

echo -n "subtracting $b from $a: "
if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi

#####################################################################
# subtract a smallnum from a bignum to produce a smallnum
a='1152921504606846978'
b=1
expected='1152921504606846977'
output=`echo "(- $a $b)" | target/psse -v 2 2>psse.log`

actual=`echo $output |\
  tail -1 |\
  sed 's/\,//g'`

echo -n "subtracting $b from $a: "
if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi


#####################################################################
# subtract a bignum  from a smallnum to produce a negstive smallnum
# (just over the boundary)
a=1
b=1152921504606846977
expected='-1152921504606846976'
output=`echo "(- $a $b)" | target/psse -v 2 2>psse.log`

actual=`echo $output |\
  tail -1 |\
  sed 's/\,//g'`

echo -n "subtracting $b from $a: "
if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi

#####################################################################
# subtract a bignum from a bignum to produce a bignum
a=20000000000000000000
b=10000000000000000000
expected=10000000000000000000
output=`echo "(- $a $b)" | target/psse -v 2 2>psse.log`

actual=`echo $output |\
  tail -1 |\
  sed 's/\,//g'`

echo -n "subtracting $b from $a: "
if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi

