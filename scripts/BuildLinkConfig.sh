#!/bin/bash
# BuildLinkConfig.sh
#
# Copyright (C) 2017-2026 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details

set -e

Top=$1
Device=$2
CLKNAME=$3

INPUTS=$(cat $Top | grep VL_IN | sed -n '/VL_INOUT/!p' | sed -n '/__/!p')
OUTPUTS=$(cat $Top | grep VL_OUT | sed -n '/__/!p')

for IN in $INPUTS; do
  NOPAREN=$(sed 's/.*(\(.*\))/\1/' <<<$IN)
  NOPAREN2=$(echo $NOPAREN | sed 's/)//')
  REMDEPTH=$(echo $NOPAREN2 | sed 's/\[[0-9]*\]//')
  SIGNAME=$(echo $REMDEPTH | sed "s/,/ /g" | awk '{print $1}' | sed "s/&//g")
  echo "link_${SIGNAME} = configureLink(\"${SIGNAME}\", \"0ns\", new Event::Handler<VerilatorSST${Device}, &VerilatorSST${Device}::handle_${SIGNAME}>(this));"
  if [[ "${SIGNAME}" == "${CLKNAME}" ]]; then
    # the clock link is wired only when the clock comes from link events
    echo "if( SelfClock ) {"
    echo "  if( nullptr != link_${SIGNAME} ) {"
    echo "    output->fatal( CALL_INFO, -1, \"Error: selfClock is set, but the clock port ${SIGNAME} is connected; leave it unconnected or unset selfClock\n\" );"
    echo "  }"
    echo "} else if( nullptr == link_${SIGNAME} ) {"
    echo "  output->fatal( CALL_INFO, -1, \"Error: was unable to configureLink link_${SIGNAME}\n\" );"
    echo "}"
  else
    echo "if( nullptr == link_${SIGNAME} ) {"
    echo "  output->fatal( CALL_INFO, -1, \"Error: was unable to configureLink link_${SIGNAME}\n\" );"
    echo "}"
  fi
done

#-- Generate all the output signals
for OUT in $OUTPUTS; do
  NOPAREN=$(sed 's/.*(\(.*\))/\1/' <<<$OUT)
  NOPAREN2=$(echo $NOPAREN | sed 's/)//')
  REMDEPTH=$(echo $NOPAREN2 | sed 's/\[[0-9]*\]//')
  SIGNAME=$(echo $REMDEPTH | sed "s/,/ /g" | awk '{print $1}' | sed "s/&//g")
  echo "link_${SIGNAME} = configureLink(\"${SIGNAME}\", \"0ns\", new Event::Handler<VerilatorSST${Device}, &VerilatorSST${Device}::handle_${SIGNAME}>(this));"
  echo "if( nullptr == link_${SIGNAME} ) {"
  echo "  output->fatal( CALL_INFO, -1, \"Error: was unable to configureLink link_${SIGNAME}\n\" );"
  echo "}"
done

# -- EOF
