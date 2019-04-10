#!/bin/bash

for adaptativeInterval in 0 1 # adaptativeInterval
do
	for cthresh in 0.5 1.0 1.5 2.0 # ctresh
	do
		for maxk in 1 2 3 # maxk
		do
			for interval in 6 12 24 48 96 # interval
			do
				for seed in 22 14 30 34 35 13 32 23 17 9 31 18 15 2 11 19 33 7 26 24 8 21 25 20 10 16 1 27 28 12 6 5 3 29 4 # seed
				do
					fileName="_SIM_ANALYSIS/readingtype_1_time_1200_adaptativeInterval_${adaptativeInterval}_cthresh_${cthresh}_maxk_${maxk}_interval_${interval}_seed_${seed}_khopca_1.txt"
					param="\"firefly_dynamic_clustering-example --readingtype=1 --time=1200 --adaptativeInterval=$adaptativeInterval --cthresh=$cthresh --maxk=$maxk --interval=$interval --seed=$seed --khopca=1\""
					./waf --run "firefly_dynamic_clustering-example --readingtype=1 --time=1200 --adaptativeInterval=$adaptativeInterval --cthresh=$cthresh --maxk=$maxk --interval=$interval --seed=$seed --khopca=1" > $fileName
				done
			done
		done
	done
done
 


