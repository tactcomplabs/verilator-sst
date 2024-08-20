#!/bin/bash
# BuildLinkConfig.sh
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details

set -e

Top=$1
Device=$2

INPUTS=$(cat $Top | grep VL_IN | sed -n '/VL_INOUT/!p' | sed -n '/__/!p')
OUTPUTS=$(cat $Top | grep VL_OUT | sed -n '/__/!p')

for IN in $INPUTS; do
  NOPAREN=$(sed 's/.*(\(.*\))/\1/' <<<$IN)
  NOPAREN2=$(echo $NOPAREN | sed 's/)//')
  REMDEPTH=$(echo $NOPAREN2 | sed 's/\[[0-9]*\]//')
  SIGNAME=$(echo $REMDEPTH | sed "s/,/ /g" | awk '{print $1}' | sed "s/&//g")
  echo "link_${SIGNAME} = configureLink(\"${SIGNAME}\", \"0ns\", new Event::Handler<VerilatorSST${Device}>(this, &VerilatorSST${Device}::handle_${SIGNAME}));"
  echo "if( nullptr == link_${SIGNAME} ) {"
  echo "  output->fatal( CALL_INFO, -1, \"Error: was unable to configureLink link_${SIGNAME}\n\" );"
  echo "}"
done

#-- Generate all the output signals
for OUT in $OUTPUTS; do
  NOPAREN=$(sed 's/.*(\(.*\))/\1/' <<<$OUT)
  NOPAREN2=$(echo $NOPAREN | sed 's/)//')
  REMDEPTH=$(echo $NOPAREN2 | sed 's/\[[0-9]*\]//')
  SIGNAME=$(echo $REMDEPTH | sed "s/,/ /g" | awk '{print $1}' | sed "s/&//g")
  echo "link_${SIGNAME} = configureLink(\"${SIGNAME}\", \"0ns\", new Event::Handler<VerilatorSST${Device}>(this, &VerilatorSST${Device}::handle_${SIGNAME}));"
  echo "if( nullptr == link_${SIGNAME} ) {"
  echo "  output->fatal( CALL_INFO, -1, \"Error: was unable to configureLink link_${SIGNAME}\n\" );"
  echo "}"
done

# -- EOF
