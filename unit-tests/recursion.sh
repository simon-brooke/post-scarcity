#!/bin/bash

expected='nil3628800'
actual=`target/psse 2>/dev/null <<EOF
(progn
  (set! fact
    (lambda (n)
    (cond ((= n 1) 1)
      (t (* n (fact (- n 1)))))))
  nil)
(fact 10)
EOF`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
    exit 0
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi
