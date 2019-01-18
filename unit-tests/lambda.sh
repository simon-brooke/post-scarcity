#!/bin/bash

expected='(lambda (l) l) (1 2 3 4 5 6 7 8 9 10)'
output=`target/psse 2>/dev/null <<EOF
(set! list (lambda (l) l))
(list '(1 2 3 4 5 6 7 8 9 10))
EOF`
actual=`echo $output | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
    exit 0
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi
