#!/bin/bash

cat $1 | grep "time:" | sed -e "s/::/:0:/g" > .tempFile

ADAPTATIVEINTERVAL=$(./getNamedFieldFromMyTrace.sh $1 "adaptativeInterval")
CTHRESH=$(./getNamedFieldFromMyTrace.sh  $1 "cthresh")
MAXK=$(./getNamedFieldFromMyTrace.sh  $1 "maxk")
INTERVAL=$(./getNamedFieldFromMyTrace.sh  $1 "interval")
SEED=$(./getNamedFieldFromMyTrace.sh $1 "seed")

#echo "" | awk '{print "\t", "qosb", "\t", "ei", "\t", "ae", "\t", "re", "\t", "uc", "\t", "ln", "\t", "dv", "\t", "amp", "\t", "ch", "\t", "ir" }' > .finalFile

#awk ' \
awk -v adap="$ADAPTATIVEINTERVAL" -v cth="$CTHRESH" -v mk="$MAXK" -v intt="$INTERVAL" -v sd="$SEED" ' BEGIN { FS=":"; q="\""; p="p" }
{
dvn=split($16,dv,",");
if (dvn > 0) {
dvsum=0;
for (i=0; i<dvn; i++)
  dvsum += dv[i];
dvmid = dvsum/dvn;
} else {
dvmid = 0;
}
ampn=split($18,amp,","); 
if (ampn > 0) {
ampsum=0;
for (i=0; i<ampn; i++)
  ampsum += amp[i];
ampmid = ampsum / ampn;
} else {
ampmid=0;
}

hopsn=split($26,hops,",");
if (hopsn<4) {
h4 = 0;
} else {
h4 = hops[4];
}

if (hopsn<3) {
h3 = 0;
} else {
h3 = hops[3];
}

if (hopsn<2) {
h2 = 0;
} else {
h2 = hops[2];
}

if (hopsn<1) {
h1 = 0;
} else {
h1 = hops[1];
}



print $2,"\t", q p adap q, "\t", q p cth q, "\t", q p mk q, "\t", q p intt q, "\t", sd, "\t",  $4, "\t", $6, "\t", $8, "\t", $10, "\t", $12, "\t", $14, "\t", dvmid, "\t", ampmid, "\t", $22, "\t", $24, "\t", h1, "\t", h2, "\t", h3, "\t", h4} \
' .tempFile > .finalFile

cat .finalFile | sed -e "1d"
