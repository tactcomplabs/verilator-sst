#!/bin/bash
# BuildPortDef.sh
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details

# cannot use set -e because some greps are expected to fail
Top=$1

INPUTS=$(cat $Top | grep VL_IN | sed -n '/VL_INOUT/!p' | sed -n '/__/!p')
OUTPUTS=$(cat $Top | grep VL_OUT)
SAFE_OUTPUTS=$(echo $OUTPUTS | tr ' ' '\n' | sed -n '/__/!p')

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
    SAFE_OUTPUTS="${SAFE_OUTPUTS//"$port_out"/}"
    SAFE_OUTPUTS="${SAFE_OUTPUTS//"$port_en"/}"
    INOUTS="$INOUTS $port"
  fi
done

for IN in $INPUTS; do
  NOPAREN=$(sed 's/.*(\(.*\))/\1/' <<<$IN)
  NOPAREN2=$(echo $NOPAREN | sed 's/)//')
  REMDEPTH=$(echo $NOPAREN2 | sed 's/\[[0-9]*\]//')
  SIGNAME=$(echo $REMDEPTH | sed "s/,/ /g" | awk '{print $1}' | sed "s/&//g")
  echo "{\"$SIGNAME\", \"Input Port\", {\"SST::VerilatorSST::PortEvent\"} },"
done

#-- Generate all the output signals
for OUT in $SAFE_OUTPUTS; do
  NOPAREN=$(sed 's/.*(\(.*\))/\1/' <<<$OUT)
  NOPAREN2=$(echo $NOPAREN | sed 's/)//')
  REMDEPTH=$(echo $NOPAREN2 | sed 's/\[[0-9]*\]//')
  SIGNAME=$(echo $REMDEPTH | sed "s/,/ /g" | awk '{print $1}' | sed "s/&//g")
  echo "{\"$SIGNAME\", \"Output port\", {\"SST::VerilatorSST::PortEvent\"} },"
done

#-- Generate all the inout signals
for INOUT in $INOUTS; do
  NOPAREN=$(sed 's/.*(\(.*\))/\1/' <<<$INOUT)
  NOPAREN2=$(echo $NOPAREN | sed 's/)//')
  REMDEPTH=$(echo $NOPAREN2 | sed 's/\[[0-9]*\]//')
  SIGNAME=$(echo $REMDEPTH | sed "s/,/ /g" | awk '{print $1}' | sed "s/&//g")
  echo "{\"$SIGNAME\", \"Inout port\", {\"SST::VerilatorSST::PortEvent\"} },"
done

# -- EOF
