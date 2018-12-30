#!/bin/bash
#
# File:   empty-list.sh.bash
# Author: simon
#
# Created on 14-Aug-2017, 15:06:40
#

expected=nil
actual=`echo "'()" | target/psse | tail -1`

if [ "${expected}" = "${actual}" ]
then
    echo "OK"
    exit 0
else
    echo "Fail: expected '${expected}', got '${actual}'"
    exit 1
fi
