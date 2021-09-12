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


# Currently failing from here on, but it's failing in read because of
# the multiply bug. We know printing blows up at the 3 cell boundary
# because `lisp/scratchpad2.lisp` constructs a 3 cell bignum by
# repeated addition.
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
    echo "Fail: expected '${expected}', \n           got '${actual}'"
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
