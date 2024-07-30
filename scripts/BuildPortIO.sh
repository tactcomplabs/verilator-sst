#!/bin/bash
# BuildPortIO.sh
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details

Top=$1

#-- Generate all the input signals
INPUTS=`cat $Top | grep VL_IN`
OUTPUTS=`cat $Top | grep VL_OUT`

# TODO: remove std::string param from direct read/write functions

for IN in $INPUTS;do
  NOPAREN=`sed 's/.*(\(.*\))/\1/' <<< $IN`
  NOPAREN2=`echo $NOPAREN | sed 's/)//'`
  REMDEPTH=`echo $NOPAREN2 | sed 's/\[[0-9]*\]//'`
  SIGNAME=`echo $REMDEPTH | sed "s/,/ /g" | awk '{print $1}' | sed "s/&//g"`
  echo "static void DirectWrite$SIGNAME(VTop *, const std::vector<uint8_t>&);"
  echo "static std::vector<uint8_t> DirectRead$SIGNAME(VTop *);"
done;

#-- Generate all the output signals
for OUT in $OUTPUTS;do
  NOPAREN=`sed 's/.*(\(.*\))/\1/' <<< $OUT`
  NOPAREN2=`echo $NOPAREN | sed 's/)//'`
  REMDEPTH=`echo $NOPAREN2 | sed 's/\[[0-9]*\]//'`
  SIGNAME=`echo $REMDEPTH | sed "s/,/ /g" | awk '{print $1}' | sed "s/&//g"`
  echo "static void DirectWrite$SIGNAME(VTop *, const std::vector<uint8_t>&);"
  echo "static std::vector<uint8_t> DirectRead$SIGNAME(VTop *);"
done;


# -- EOF
