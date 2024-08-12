#!/bin/bash
# BuildPortHandlers.sh
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details

Top=$1
Device=$2
LINK=$3
CLKNAME=$4

#-- Generate all the input signals
INPUTS=`cat $Top | grep VL_IN | sed -n '/VL_INOUT/!p' | sed -n '/__/!p'`
OUTPUTS=`cat $Top | grep VL_OUT | sed -n '/__/!p'`
ALL="${INPUTS} ${OUTPUTS}"

#-- Generate all the inout signals
for PORT in $ALL;do
  NOPAREN=`sed 's/.*(\(.*\))/\1/' <<< $PORT`
  NOPAREN2=`echo $NOPAREN | sed 's/)//'`
  REMDEPTH=`echo $NOPAREN2 | sed 's/\[[0-9]*\]//'`
  SIGNAME=`echo $REMDEPTH | sed "s/,/ /g" | awk '{print $1}' | sed "s/&//g"`
  HANDLER_IMPL="output->fatal(CALL_INFO, -1, \"received a port event, but link handling is disabled\");"

  if (( $LINK > 0 )); then
    HANDLER_IMPL="portEventHandler(\"${SIGNAME}\", ev, link_${SIGNAME});"
  fi

  echo "void VerilatorSST$Device::handle_${SIGNAME}(SST::Event* ev){
  ${HANDLER_IMPL}
}
"
done;

# -- EOF
