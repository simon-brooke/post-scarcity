#1/bin/bash

echo "Case, Summary, Allocated, Deallocated, Not deallocated, Delta Allocated, Delta Deallocated, Delta Not Deallocated"
basecase=`echo '' | ../../target/psse 2>&1 | grep Allocation | tr -d  '[:punct:]'`
bca=`echo ${basecase} | awk '{print $4}'`
bcd=`echo ${basecase} | awk '{print $6}'`
bcn=`echo ${basecase} | awk '{print $9}'`

echo "\"Basecase\", \"${basecase}\", ${bca}, ${bcd}, ${bcn}"

while IFS= read -r form; do
    allocation=`echo ${form} | ../../target/psse 2>&1 | grep Allocation | tr -d  '[:punct:]'`
    tca=`echo ${allocation} | awk '{print $4}'`
    tcd=`echo ${allocation} | awk '{print $6}'`
    tcn=`echo ${allocation} | awk '{print $9}'`

    dca=`echo "${tca} - ${bca}" | bc`
    dcd=`echo "${tcd} - ${bcd}" | bc`
    dcn=`echo "${tcn} - ${bcn}" | bc`

    echo "\"${form}\", \"${allocation}\", ${tca}, ${tcd}, ${tcn}, ${dca}, ${dcd}, ${dcn}" 
done
