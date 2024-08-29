#!/bin/bash
# BuildPortEntry.sh
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details

# cannot use set -e because some grep is expected to fail

Top=$1
Device=$2

INPUTS=$(cat $Top | grep VL_IN | sed -n '/VL_INOUT/!p')
OUTPUTS=$(cat $Top | grep VL_OUT)

#-- check for inout port pattern:
# input  port
# output port__out
# output port__en
for port in $INPUTS; do
  port_name=$(echo $port | awk -F[\&,] '{print $2}')
  port_out=$(echo $OUTPUTS | tr ' ' '\n' | grep "${port_name}__out")
  port_en=$(echo $OUTPUTS | tr ' ' '\n' | grep "${port_name}__en")

  if [[ ! -z "$port_out" ]] && [[ ! -z "$port_en" ]]; then
    INPUTS="${INPUTS//"$port"/}"
    INOUTS="$INOUTS $port"
  fi
done

for IN in $INPUTS; do
  NOPAREN=$(sed 's/.*(\(.*\))/\1/' <<<$IN)
  NOPAREN2=$(echo $NOPAREN | sed 's/)//')
  DEPTH=$(echo $NOPAREN2 | sed 's/[][]/ /g' | awk '{print $2}')
  if [ -z "$DEPTH" ]; then
    DEPTH=1
  fi
  REMDEPTH=$(echo $NOPAREN2 | sed 's/\[[0-9]*\]//')
  SIGNAME=$(echo $REMDEPTH | sed "s/,/ /g" | awk '{print $1}' | sed "s/&//g")
  ENDBIT=$(echo $REMDEPTH | sed "s/,/ /g" | awk '{print $2}')
  STARTBIT=$(echo $REMDEPTH | sed "s/,/ /g" | awk '{print $3}' | sed "s/;//g")

  ENDBIT=$(($ENDBIT + 1))
  WIDTH=$(($ENDBIT - $STARTBIT))

  echo "{\"$SIGNAME\", SST::VerilatorSST::VPortType::V_INPUT, $WIDTH, $DEPTH, SST::VerilatorSST::VerilatorSST$Device::DirectWrite$SIGNAME, SST::VerilatorSST::VerilatorSST$Device::DirectRead$SIGNAME },"
done

#-- Generate all the output signals
for OUT in $OUTPUTS; do
  NOPAREN=$(sed 's/.*(\(.*\))/\1/' <<<$OUT)
  NOPAREN2=$(echo $NOPAREN | sed 's/)//')
  DEPTH=$(echo $NOPAREN2 | sed 's/[][]/ /g' | awk '{print $2}')
  if [ -z "$DEPTH" ]; then
    DEPTH=1
  fi
  REMDEPTH=$(echo $NOPAREN2 | sed 's/\[[0-9]*\]//')
  SIGNAME=$(echo $REMDEPTH | sed "s/,/ /g" | awk '{print $1}' | sed "s/&//g")
  ENDBIT=$(echo $REMDEPTH | sed "s/,/ /g" | awk '{print $2}')
  STARTBIT=$(echo $REMDEPTH | sed "s/,/ /g" | awk '{print $3}' | sed "s/;//g")

  ENDBIT=$(($ENDBIT + 1))
  WIDTH=$(($ENDBIT - $STARTBIT))

  echo "{\"$SIGNAME\", SST::VerilatorSST::VPortType::V_OUTPUT, $WIDTH, $DEPTH, SST::VerilatorSST::VerilatorSST$Device::DirectWrite$SIGNAME, SST::VerilatorSST::VerilatorSST$Device::DirectRead$SIGNAME },"
done

#-- Generate all the inout signals
for INOUT in $INOUTS; do
  NOPAREN=$(sed 's/.*(\(.*\))/\1/' <<<$INOUT)
  NOPAREN2=$(echo $NOPAREN | sed 's/)//')
  DEPTH=$(echo $NOPAREN2 | sed 's/[][]/ /g' | awk '{print $2}')
  if [ -z "$DEPTH" ]; then
    DEPTH=1
  fi
  REMDEPTH=$(echo $NOPAREN2 | sed 's/\[[0-9]*\]//')
  SIGNAME=$(echo $REMDEPTH | sed "s/,/ /g" | awk '{print $1}' | sed "s/&//g")
  ENDBIT=$(echo $REMDEPTH | sed "s/,/ /g" | awk '{print $2}')
  STARTBIT=$(echo $REMDEPTH | sed "s/,/ /g" | awk '{print $3}' | sed "s/;//g")

  ENDBIT=$(($ENDBIT + 1))
  WIDTH=$(($ENDBIT - $STARTBIT))

  echo "{\"$SIGNAME\", SST::VerilatorSST::VPortType::V_INOUT, $WIDTH, $DEPTH, SST::VerilatorSST::VerilatorSST$Device::DirectWrite$SIGNAME, SST::VerilatorSST::VerilatorSST$Device::DirectRead$SIGNAME },"
done

# -- EOF
