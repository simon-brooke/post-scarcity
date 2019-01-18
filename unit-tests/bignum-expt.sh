#!/bin/bash

#####################################################################
# last 'smallnum' value:
# sbcl calculates (expt 2 59) => 576460752303423488
expected='576460752303423488'

output=`target/psse <<EOF
(progn
  (set! expt (lambda
              (n x)
              (cond
                ((= x 1) n)
                (t (* n (expt n (- x 1)))))))
  nil)
  (expt 2 59)
EOF`

actual=`echo "$output" | tail -1 | sed 's/\,//g'`

echo -n "(expt 2 59): "
if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi

#####################################################################
# first 'bignum' value (right on the boundary):
# sbcl calculates (expt 2 60) => 1152921504606846976
expected='1152921504606846976'

output=`target/psse <<EOF
(progn
  (set! expt (lambda
              (n x)
              (cond
                ((= x 1) n)
                (t (* n (expt n (- x 1)))))))
  nil)
  (expt 2 60)
EOF`

actual=`echo "$output" | tail -1 | sed 's/\,//g'`

echo -n "(expt 2 60): "
if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi

#####################################################################
# second 'bignum' value (definitely a bignum):
# sbcl calculates (expt 2 61) => 2305843009213693952
expected='2305843009213693952'

output=`target/psse <<EOF
(progn
  (set! expt (lambda
              (n x)
              (cond
                ((= x 1) n)
                (t (* n (expt n (- x 1)))))))
  nil)
  (expt 2 61)
EOF`

actual=`echo "$output" | tail -1 | sed 's/\,//g'`

echo -n "(expt 2 61): "
if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi



# sbcl calculates (expt 2 64) => 18446744073709551616
expected='18446744073709551616'

output=`target/psse <<EOF
(progn
  (set! expt (lambda
              (n x)
              (cond
                ((= x 1) n)
                (t (* n (expt n (- x 1)))))))
  nil)
  (expt 2 64)
EOF`

actual=`echo "$output" | tail -1 | sed 's/\,//g'`

echo -n "(expt 2 64): "
if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi

# sbcl calculates (expt 2 65) => 36893488147419103232
expected='36893488147419103232'

output=`target/psse <<EOF
(progn
  (set! expt (lambda
              (n x)
              (cond
                ((= x 1) n)
                (t (* n (expt n (- x 1)))))))
  nil)
  (expt 2 65)
EOF`

actual=`echo "$output" | tail -1 | sed 's/\,//g'`

echo -n "(expt 2 65): "
if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi
exit 0
