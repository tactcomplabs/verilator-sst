#!/bin/bash
# BuildPortEntry.sh
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details

Top=$1
Device=$2

#-- Generate all the input signals
INPUTS=`cat $Top | grep VL_IN`
OUTPUTS=`cat $Top | grep VL_OUT`

for IN in $INPUTS;do
  NOPAREN=`sed 's/.*(\(.*\))/\1/' <<< $IN`
  SIGNAME=`echo $NOPAREN | sed "s/,/ /g" | awk '{print $1}' | sed "s/&//g"`
  ENDBIT=`echo $NOPAREN | sed "s/,/ /g" | awk '{print $2}'`
  STARTBIT=`echo $NOPAREN | sed "s/,/ /g" | awk '{print $3}' | sed "s/;//g"`

  ENDBIT=$(($ENDBIT + 1))
  WIDTH=$(($ENDBIT - $STARTBIT))

  echo "{\"$SIGNAME\", SST::VerilatorSST::VPortType::V_INPUT, $WIDTH, SST::VerilatorSST::VerilatorSST$Device::DirectWrite$SIGNAME, SST::VerilatorSST::VerilatorSST$Device::DirectRead$SIGNAME },"
done;

#-- Generate all the output signals
for OUT in $OUTPUTS;do
  NOPAREN=`sed 's/.*(\(.*\))/\1/' <<< $OUT`
  SIGNAME=`echo $NOPAREN | sed "s/,/ /g" | awk '{print $1}' | sed "s/&//g"`
  ENDBIT=`echo $NOPAREN | sed "s/,/ /g" | awk '{print $2}'`

  ENDBIT=$(($ENDBIT + 1))
  WIDTH=$(($ENDBIT - $STARTBIT))

  echo "{\"$SIGNAME\", SST::VerilatorSST::VPortType::V_OUTPUT, $WIDTH, SST::VerilatorSST::VerilatorSST$Device::DirectWrite$SIGNAME, SST::VerilatorSST::VerilatorSST$Device::DirectRead$SIGNAME },"
done;


# -- EOF
