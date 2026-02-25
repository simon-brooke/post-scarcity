#!/bin/bash

result=0

# echo -n "$0: let with two bindings, one form in body..."
# expected='11'
# actual=`echo "(let ((a . 5)(b . 6)) (+ a b))" | target/psse 2>/dev/null | tail -1`

# if [ "${expected}" = "${actual}" ]
# then
#     echo "OK"
# else
#     echo "Fail: expected '$expected', got '$actual'"
#     result=`echo "${result} + 1" | bc`
# fi

# echo -n "$0: let with two bindings, two forms in body..."
# expected='1'
# actual=`echo "(let ((a . 5)(b . 6)) (+ a b) (- b a))" | target/psse 2>/dev/null | tail -1`

# if [ "${expected}" = "${actual}" ]
# then
#     echo "OK"
# else
#     echo "Fail: expected '$expected', got '$actual'"
#     result=`echo "${result} + 1" | bc`
# fi

exit ${result}