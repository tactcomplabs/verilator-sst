#!/bin/bash
# BuildPortDef.sh
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details

Top=$1

#-- Generate all the input signals
INPUTS=`cat $Top | grep VL_IN | sed -n '/VL_INOUT/!p'`
OUTPUTS=`cat $Top | grep VL_OUT`
INOUTS=$(cat $Top | grep VL_INOUT)

for IN in $INPUTS;do
  NOPAREN=`sed 's/.*(\(.*\))/\1/' <<< $IN`
  NOPAREN2=`echo $NOPAREN | sed 's/)//'`
  REMDEPTH=`echo $NOPAREN2 | sed 's/\[[0-9]*\]//'`
  SIGNAME=`echo $REMDEPTH | sed "s/,/ /g" | awk '{print $1}' | sed "s/&//g"`
  echo "{\"$SIGNAME\", \"Input Port\", {\"SST::VerilatorSST::PortEvent\"} },"
done;

#-- Generate all the output signals
for OUT in $OUTPUTS;do
  NOPAREN=`sed 's/.*(\(.*\))/\1/' <<< $OUT`
  NOPAREN2=`echo $NOPAREN | sed 's/)//'`
  REMDEPTH=`echo $NOPAREN2 | sed 's/\[[0-9]*\]//'`
  SIGNAME=`echo $REMDEPTH | sed "s/,/ /g" | awk '{print $1}' | sed "s/&//g"`
  echo "{\"$SIGNAME\", \"Output port\", {\"SST::VerilatorSST::PortEvent\"} },"
done;

#-- Generate all the inout signals
for INOUT in $INOUTS;do
  NOPAREN=`sed 's/.*(\(.*\))/\1/' <<< $INOUT`
  NOPAREN2=`echo $NOPAREN | sed 's/)//'`
  REMDEPTH=`echo $NOPAREN2 | sed 's/\[[0-9]*\]//'`
  SIGNAME=`echo $REMDEPTH | sed "s/,/ /g" | awk '{print $1}' | sed "s/&//g"`
  echo "{\"$SIGNAME\", \"Inout port\", {\"SST::VerilatorSST::PortEvent\"} },"
done;

# -- EOF
