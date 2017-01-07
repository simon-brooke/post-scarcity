#!/bin/bash

# Early stage unit test runner for post-scarcity software environment.
# Uses bash to run every file in the unit-tests subdirectory; expects these
# to return 0 on success, anything else on fail.

# (c) 2017 Simon Brooke <simon@journeyman.cc>
# Licensed under GPL version 2.0, or, at your option, any later version.

tests=0
pass=0
fail=0

for file in unit-tests/*
do
    echo -n "${file} => "
    bash ${file}

    if [ $? -eq 0 ]
    then
	pass=$((${pass} + 1))
    else
	fail=$((${fail} + 1))
    fi

    tests=$((${tests} + 1))
done

echo
echo "Tested ${tests}, passed ${pass}, failed ${fail}"

exit ${fail}

