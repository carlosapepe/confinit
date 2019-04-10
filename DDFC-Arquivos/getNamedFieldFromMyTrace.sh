#!/bin/bash

DELIM=":"
FIELD="$2"

STRING="$1"

echo ${STRING#*$FIELD$DELIM} | cut -d "$DELIM" -f1
