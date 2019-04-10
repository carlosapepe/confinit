#!/bin/bash

dirP="./_SIMULATION_NEST"


FILENAME="GLOBAL_FINISH_TABLE_FOR_R_2"

echo -n -e "adap\tcth\tmk\tint\tsd\tdur\tqtd" > $FILENAME
echo "" >> $FILENAME

totalF=$(ls $dirP | wc -l)
i=0

for f in $(ls $dirP)
do
	i=$(( $i + 1 ))
	./_normalizeFINISHLineOfFileForR.sh $dirP/$f >> $FILENAME
	echo "$i / $totalF"
done
