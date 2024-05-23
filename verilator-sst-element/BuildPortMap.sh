#!/bin/bash
# BuildPortMap.sh
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details

Top=$1

#-- Generate all the input signals
INPUTS=`cat $Top | grep VL_IN`
OUTPUTS=`cat $Top | grep VL_OUT`

IDX=0

for IN in $INPUTS;do
  NOPAREN=`sed 's/.*(\(.*\))/\1/' <<< $IN`
  SIGNAME=`echo $NOPAREN | sed "s/,/ /g" | awk '{print $1}' | sed "s/&//g"`
  echo "{\"$SIGNAME\", $IDX },"
  IDX=$(($IDX + 1))
done;

#-- Generate all the output signals
for OUT in $OUTPUTS;do
  NOPAREN=`sed 's/.*(\(.*\))/\1/' <<< $OUT`
  SIGNAME=`echo $NOPAREN | sed "s/,/ /g" | awk '{print $1}' | sed "s/&//g"`
  echo "{\"$SIGNAME\", $IDX },"
  IDX=$(($IDX + 1))
done;


# -- EOF