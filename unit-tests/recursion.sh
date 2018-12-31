#!/bin/bash

expected='nil 3,628,800'
output=`target/psse 2>/dev/null <<EOF
(progn
  (set! fact
    (lambda (n)
    (cond ((= n 1) 1)
      (t (* n (fact (- n 1)))))))
  nil)
(fact 10)
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
