#!/bin/bash

READINGTYPE="2"
TIME="1200"
ADAPTATIVEINTERVAL="0"
CTHRESH="1.0"
MAXK="3"
INTERVAL="12"

TIME_STEP=10

dirP="./_SIMULATION_NEST"

str="readingtype:$READINGTYPE:time:$TIME:adaptativeInterval:$ADAPTATIVEINTERVAL:cthresh:$CTHRESH:maxk:$MAXK:interval:$INTERVAL:seed:"

FIELD="beacons"

echo ">> str:: $str"

#for i in $(ls $dirP | grep $str)
#do

echo "" > .tempData

for j in $(seq 0 10 120)
do
	for line in $(cat $dirP/$str*  | grep "time:$j:")
	do
		getField $line $FIELD ":"
	done >> .tempData
	conj=$(cat .tempData | sed -e ':a;N;$!ba;s/\n/,/g' | sed -e "s/^,//g")
	R CMD BATCH "--args inpt=c($conj)" _getRTest.R R.out

	if [ $(cat R.out | grep -c "data are essentially constant") -eq 1 ]
	then
		mean=$(echo $conj | cut -d"," -f1)
		lower=$mean
		higher=$mean
	else
		mean=$(cat R.out  | grep -a1 "mean" | tail -n1 | fmt -1 | tr -s " " | head -n1 | sed -e "s/ //g")
		lower=$(cat R.out  | grep -a1 "confidence interval" | tail -n1 | fmt -1 | head -n1 | sed -e "s/ //g")
		higher=$(cat R.out  | grep -a1 "confidence interval" | tail -n1 | fmt -1 | tail -n1| sed -e "s/ //g")
	fi

	if [[ "$lower" == "NaN" ]]
	then
		lower=$mean
		higher=$mean
	fi

	echo "$j $lower $mean $higher"

done
#done
