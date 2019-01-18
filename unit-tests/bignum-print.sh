#!/bin/bash

#####################################################################
# large number, not actally a bignum
expected='576460752303423488'
output=`echo "(progn (print $expected) nil)" | target/psse -v 2 2>psse.log`

actual=`echo $output |\
  sed 's/\,//g' |\
  sed 's/[^0-9]*\([0-9]*\).*/\1/'`

echo -n "printing $expected: "
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
# right on the boundary
expected='1152921504606846976'
output=`echo "(progn (print $expected) nil)" | target/psse -v 2 2>psse.log`

actual=`echo $output |\
  sed 's/\,//g' |\
  sed 's/[^0-9]*\([0-9]*\).*/\1/'`

echo -n "printing $expected: "
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
# definitely a bignum
expected='1152921504606846977'
output=`echo "(progn (print $expected) nil)" | target/psse -v 2 2>psse.log`

actual=`echo $output |\
  sed 's/\,//g' |\
  sed 's/[^0-9]*\([0-9]*\).*/\1/'`

echo -n "printing $expected: "
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
# Just on the three cell boundary
expected='1329227995784915872903807060280344576'
output=`echo "(progn (print $expected) nil)" | target/psse -v 2 2>psse.log`

actual=`echo $output |\
  sed 's/\,//g' |\
  sed 's/[^0-9]*\([0-9]*\).*/\1/'`

echo -n "printing $expected: "
if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi

exit 0

#####################################################################
# definitely a three cell bignum
expected='1329227995784915872903807060280344577'
output=`echo "(progn (print $expected) nil)" | target/psse -v 2 2>psse.log`

actual=`echo $output |\
  sed 's/\,//g' |\
  sed 's/[^0-9]*\([0-9]*\).*/\1/'`

echo -n "printing $expected: "
if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi

exit 0
