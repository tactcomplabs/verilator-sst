#!/bin/bash
# BuildPortHandlers.sh
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details

set -e

Top=$1
Device=$2
LINK=$3
CLKNAME=$4

INPUTS=$(cat $Top | grep VL_IN | sed -n '/VL_INOUT/!p' | sed -n '/__/!p')
OUTPUTS=$(cat $Top | grep VL_OUT | sed -n '/__/!p')

#-- Generate all the input signals
for INPUT in $INPUTS; do
  NOPAREN=$(sed 's/.*(\(.*\))/\1/' <<<$INPUT)
  NOPAREN2=$(echo $NOPAREN | sed 's/)//')
  REMDEPTH=$(echo $NOPAREN2 | sed 's/\[[0-9]*\]//')
  SIGNAME=$(echo $REMDEPTH | sed "s/,/ /g" | awk '{print $1}' | sed "s/&//g")
  HANDLER_IMPL="output->fatal(CALL_INFO, -1, \"received a input event, but link handling is disabled\");"

  if (($LINK > 0)); then
    HANDLER_IMPL="const PortEvent * portEvent = static_cast<const PortEvent *>(ev);
  if(portEvent->getAction() == PortEventAction::WRITE) {
    if( portEvent->getAtTick() > 0 ){
      writePortAtTick(\"${SIGNAME}\",portEvent->getPacket(),portEvent->getAtTick());
    }else{
      writePort(\"${SIGNAME}\",portEvent->getPacket());
    }
    delete portEvent;
    return;
  }

  if(portEvent->getAction() == PortEventAction::READ) {
    const std::vector<uint8_t> packet = readPort(\"${SIGNAME}\");
    PortEvent * respPortEvent = new PortEvent(packet);
    link_${SIGNAME}->send(respPortEvent);
    delete portEvent;
    return;
  }

  output->fatal(CALL_INFO, -1, \"received port event with unrecognized action. portName=${SIGNAME} action=%u\n\",static_cast<uint8_t>(portEvent->getAction()));"
  fi

  if [[ "${SIGNAME}" == "${CLKNAME}" ]]; then
    HANDLER_IMPL="//clock handler
  const PortEvent * portEvent = static_cast<const PortEvent *>(ev);
  pollWriteQueue();
  writePort(\"${SIGNAME}\",portEvent->getPacket());
  ContextP->timeInc(1);
  delete portEvent;"
  fi

  echo "void VerilatorSST${Device}::handle_${SIGNAME}(SST::Event* ev){
  ${HANDLER_IMPL}
}
"
done

#-- Generate all the output signals
for OUTPUT in $OUTPUTS; do
  NOPAREN=$(sed 's/.*(\(.*\))/\1/' <<<$OUTPUT)
  NOPAREN2=$(echo $NOPAREN | sed 's/)//')
  REMDEPTH=$(echo $NOPAREN2 | sed 's/\[[0-9]*\]//')
  SIGNAME=$(echo $REMDEPTH | sed "s/,/ /g" | awk '{print $1}' | sed "s/&//g")
  HANDLER_IMPL="output->fatal(CALL_INFO, -1, \"received a input event, but link handling is disabled\");"

  if (($LINK > 0)); then
    HANDLER_IMPL="const PortEvent * portEvent = static_cast<const PortEvent *>(ev);

  if(portEvent->getAction() == PortEventAction::READ) {
    const std::vector<uint8_t> packet = readPort(\"${SIGNAME}\");
    PortEvent * respPortEvent = new PortEvent(packet);
    link_${SIGNAME}->send(respPortEvent);
    delete portEvent;
    return;
  }

  output->fatal(CALL_INFO, -1, \"received port event with unrecognized action. portName=${SIGNAME} action=%u\n\",static_cast<uint8_t>(portEvent->getAction()));"
  fi

  echo "void VerilatorSST${Device}::handle_${SIGNAME}(SST::Event* ev){
  ${HANDLER_IMPL}
}
"
done

# -- EOF
