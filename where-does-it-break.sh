#!/bin/bash

# Not really a unit test, but a check to see where bignum addition breaks

broken=0
i=11529215046068469750
# we've already proven we can successfullu get up to here
increment=1

while [ $broken -eq "0" ]
do
  expr="(+ $i $increment)"
  # Use sbcl as our reference implementation...
  expected=`echo "$expr" | sbcl --noinform | grep -v '*'`
  actual=`echo "$expr" | target/psse | tail -1 | sed 's/\,//g'`

  echo -n "adding $increment to $i: "

  if [ "${expected}" = "${actual}" ]
  then
      echo "OK"
  else
      echo "Fail: expected '${expected}', got '${actual}'"
      broken=1
      exit 1
  fi

  i=$expected
done
