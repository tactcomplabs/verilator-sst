#!/bin/bash
# BuildPortMap.sh
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details

Top=$1

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

#-- aggregate ports list
ALL="$INPUTS $OUTPUTS $INOUTS"

#-- generate all signals
IDX=0
for port in $ALL; do
  NOPAREN=$(sed 's/.*(\(.*\))/\1/' <<<$port)
  NOPAREN2=$(echo $NOPAREN | sed 's/)//')
  REMDEPTH=$(echo $NOPAREN2 | sed 's/\[[0-9]*\]//')
  SIGNAME=$(echo $REMDEPTH | sed "s/,/ /g" | awk '{print $1}' | sed "s/&//g")
  echo "{\"$SIGNAME\", $IDX },"
  IDX=$(($IDX + 1))
done

# -- EOF
