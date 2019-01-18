#!/bin/bash

expected='"god yzal eht revo depmuj xof nworb kciuq ehT"'
actual=`echo '(reverse "The quick brown fox jumped over the lazy dog")' | target/psse | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi

expected='(1,024 512 256 128 64 32 16 8 4 2)'
actual=`echo "(reverse '(2 4 8 16 32 64 128 256 512 1024))" | target/psse | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi

expected='esrever'
actual=`echo "(reverse 'reverse)" | target/psse | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
    exit 0
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi

