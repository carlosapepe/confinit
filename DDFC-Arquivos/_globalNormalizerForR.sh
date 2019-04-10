#!/bin/bash

dirP="./_SIMULATION_NEST"

totalF=$(ls $dirP | wc -l)
i=0

FILENAME="GLOBAL_TABLE_FOR_R_2"

echo "" | awk '{print "time", "\t", "adap", "\t", "cth", "\t", "mk", "\t", "int", "\t", "sd", "\t", "qosb", "\t", "ei", "\t", "ae", "\t", "re", "\t", "uc", "\t", "ln", "\t", "dv", "\t", "amp", "\t", "ch", "\t", "ir", "\t", "1h", "\t", "2h", "\t", "3h", "\t", "4h" }' > $FILENAME

for f in $(ls $dirP)
do
	i=$(( $i + 1 ))
	./_normalizeFileForR.sh $dirP/$f # >> $FILENAME
	echo "$i / $totalF"
done
