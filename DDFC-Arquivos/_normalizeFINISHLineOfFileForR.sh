#!/bin/bash

line=$(cat $1 | grep "FINISH:")
line=$(./getNamedFieldFromMyTrace.sh $line "number_of_cluster_heads_that_remained_X_turns")

ADAPTATIVEINTERVAL=$(./getNamedFieldFromMyTrace.sh $1 "adaptativeInterval")
CTHRESH=$(./getNamedFieldFromMyTrace.sh  $1 "cthresh")
MAXK=$(./getNamedFieldFromMyTrace.sh  $1 "maxk")
INTERVAL=$(./getNamedFieldFromMyTrace.sh  $1 "interval")
SEED=$(./getNamedFieldFromMyTrace.sh $1 "seed")

n=0
for i in $(echo $line | sed -e "s/,/ /g" | fmt -1)
do

	n=$(( $n + 1 ))

	# duration qtd
	echo -n -e "p$ADAPTATIVEINTERVAL\tp$CTHRESH\tp$MAXK\tp$INTERVAL\t$SEED"
	echo -n -e "\t$n\t$i"
	echo ""

done

