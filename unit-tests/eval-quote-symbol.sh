#!/bin/bash

expected='<Special form: ((:primitive . t) (:name . cond) (:documentation . "`(cond clauses...)`: Conditional evaluation, `clauses` is a sequence of lists of forms such that if evaluating the first form in any clause returns non-`nil`, the subsequent forms in that clause will be evaluated and the value of the last returned; but any subsequent clauses will not be evaluated."))>'
actual=`echo "(eval 'cond)" | target/psse 2>/dev/null | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi
