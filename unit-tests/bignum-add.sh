#!/bin/bash

#####################################################################
# add two large numbers, not actally bignums to produce a smallnum
# (right on the boundary)
a=1152921504606846975
b=1
c=`echo "$a + $b" | bc`
expected='t'
output=`echo "(= (+ $a $b) $c)" | target/psse -v 2 2>psse.log`

actual=`echo $output |\
  tail -1`

echo -n "adding $a to $b: "
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
# add two numbers, not actally bignums to produce a bignum
# (just over the boundary)
a='1152921504606846976'
b=1
c=`echo "$a + $b" | bc`
expected='t'
output=`echo "(= (+ $a $b) $c)" | target/psse -v 2 2>psse.log`

actual=`echo $output |\
  tail -1 |\
  sed 's/\,//g'`

echo -n "adding $a to $b: "
if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi

echo -n "checking a bignum was created: "
grep 'BIGNUM!' psse.log > /dev/null
if [ $? -eq "0" ]
then
    echo "OK"
else
    echo "Fail"
    exit 1
fi


#####################################################################
# add a bignum and a smallnum to produce a bignum
# (just over the boundary)
a='1152921504606846977'
b=1
c=`echo "$a + $b" | bc`
expected='t'
output=`echo "(= (+ $a $b) $c)" | target/psse -v 2 2>psse.log`

actual=`echo $output |\
  tail -1 |\
  sed 's/\,//g'`

echo -n "adding $a to $b: "
if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi

echo -n "checking a bignum was created: "
grep 'BIGNUM!' psse.log > /dev/null
if [ $? -eq "0" ]
then
    echo "OK"
else
    echo "Fail"
    exit 1
fi

#####################################################################
# add a smallnum and a bignum to produce a bignum
# (just over the boundary)
a=1
b=1152921504606846977
c=`echo "$a + $b" | bc`
expected='t'
output=`echo "(= (+ $a $b) $c)" | target/psse -v 2 2>psse.log`

actual=`echo $output |\
  tail -1 |\
  sed 's/\,//g'`

echo -n "adding $a to $b: "
if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi

echo -n "checking a bignum was created: "
grep 'BIGNUM!' psse.log > /dev/null
if [ $? -eq "0" ]
then
    echo "OK"
else
    echo "Fail"
    exit 1
fi


#####################################################################
# add two bignums to produce a bignum
a=10000000000000000000
b=10000000000000000000
c=`echo "$a + $b" | bc`
expected='t'
output=`echo "(= (+ $a $b) $c)" | target/psse -v 2 2>psse.log`

actual=`echo $output |\
  tail -1 |\
  sed 's/\,//g'`

echo -n "adding $a to $b: "
if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi

echo -n "checking a bignum was created: "
grep 'BIGNUM!' psse.log > /dev/null
if [ $? -eq "0" ]
then
    echo "OK"
else
    echo "Fail"
    exit 1
fi


#####################################################################
# add a smallnum and a two-cell bignum to produce a three-cell bignum
# (just over the boundary)
a=1
b=1329227995784915872903807060280344576
c=`echo "$a + $b" | bc`
expected='t'
output=`echo "(= (+ $a $b) $c)" | target/psse -v 2 2>psse.log`

actual=`echo $output |\
  tail -1 |\
  sed 's/\,//g'`

echo -n "adding $a to $b: "
if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi

echo -n "checking a bignum was created: "
grep 'BIGNUM!' psse.log > /dev/null
if [ $? -eq "0" ]
then
    echo "OK"
else
    echo "Fail"
    exit 1
fi


#####################################################################
# This currently fails:
# (= (+ 1 3064991081731777716716694054300618367237478244367204352)
#         3064991081731777716716694054300618367237478244367204353)
a=1
b=3064991081731777716716694054300618367237478244367204352
c=`echo "$a + $b" | bc`
expected='t'
output=`echo "(= (+ $a $b) $c)" | target/psse -v 2 2>psse.log`

actual=`echo $output |\
  tail -1 |\
  sed 's/\,//g'`

echo -n "adding $a to $b: "
if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi

echo -n "checking a bignum was created: "
grep 'BIGNUM!' psse.log > /dev/null
if [ $? -eq "0" ]
then
    echo "OK"
else
    echo "Fail"
    exit 1
fi
