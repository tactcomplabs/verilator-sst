#!/bin/bash
# BuildPortHandlers.sh
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
  echo "void VerilatorSST$Device::handle_${SIGNAME}(SST::Event* ev){"
  echo "PortEvent *p = static_cast<PortEvent*>(ev);"
  echo "// handle the message"
  echo "if( p->getAtTick() != 0x00ull ){"
  echo "writePortAtTick(\"${SIGNAME}\",p->getPacket(),p->getAtTick());"
  echo "}else{"
  echo "writePort(\"${SIGNAME}\",p->getPacket());"
  echo "}"
  echo "delete ev;"
  echo "}"
done;

#-- Generate all the output signals
for OUT in $OUTPUTS;do
  NOPAREN=`sed 's/.*(\(.*\))/\1/' <<< $OUT`
  SIGNAME=`echo $NOPAREN | sed "s/,/ /g" | awk '{print $1}' | sed "s/&//g"`
  echo "void VerilatorSST$Device::handle_${SIGNAME}(SST::Event* ev){"
  echo "// handle the message"
  echo "std::vector<uint8_t> Packet;"
  echo "Packet = readPort(\"${SIGNAME}\");"
  echo "delete ev;"
  echo "PortEvent *pe = new PortEvent(Packet);"
  echo "}"
done;


# -- EOF