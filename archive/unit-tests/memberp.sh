#!/bin/bash

result=0

expected='t'
output=`target/psse $1 <<EOF
(progn
  (set! nil? (lambda (o) (= (type o) "NIL ")))
  (set! member? 
    (lambda
        (item collection)
        (cond
          ((nil? collection) nil)
          ((= item (car collection)) t)
          (t (member? item (cdr collection))))))
  (member? 1 '(1 2 3 4)))
EOF`
actual=`echo $output | tail -1`

echo -n "$0 $1: (member? 1 '(1 2 3 4))... "

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi

expected='t'
output=`target/psse $1 <<EOF
(progn
  (set! nil? (lambda (o) (= (type o) "NIL ")))
  (set! member? 
    (lambda
      (item collection)
      (cond
        ((nil? collection) nil)
        ((= item (car collection)) t)
        (t (member? item (cdr collection))))))
  (member? 4 '(1 2 3 4)))
EOF`
actual=`echo $output | tail -1`

echo -n "$0: (member? 4 '(1 2 3 4))... "

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi


expected='nil'
output=`target/psse $1 <<EOF
(progn
  (set! nil? (lambda (o) (= (type o) "NIL ")))
  (set! member? 
    (lambda
      (item collection)
      ;; (progn (print (list "In member; collection is:" collection)) (println))
      (cond
        ((nil? collection) nil)
        ((= item (car collection)) t)
        (t (member? item (cdr collection))))))
  (member? 5 '(1 2 3 4)))
EOF`
actual=`echo $output | tail -1`

echo -n "$0: (member? 5 '(1 2 3 4))... "

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi

expected='nil'
output=`target/psse $1 -s100<<EOF
(progn
  (set! nil? (lambda (o) (= (type o) "NIL ")))
  (set! member? 
    (lambda
      (item collection)
      ;; (print (list "in member?: " 'item item 'collection collection) *log*)(println *log*)
      (cond
        ((nil? collection) nil)
        ((= item (car collection)) t)
        (t (member? item (cdr collection))))))
  (member? 5 '(1 2 3 4)))
EOF`
actual=`echo $output | tail -1`

echo -n "$0: (member? 5 '(1 2 3 4)) with stack limit... "

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
else
    echo "Fail: expected '${expected}', got '${actual}'"
    result=`echo "${result} + 1" | bc`
fi

exit $result
