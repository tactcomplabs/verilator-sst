#!/bin/bash
# BuiltPortIOImpls.sh
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

build_write () {
  SIGNAME=$1
  WIDTH=$2

  if [ $WIDTH -lt 9 ]; then
    # less than a 8 bits
    echo "// less than 8 bit"
    echo "SignalHelper S($WIDTH);"
    echo "T->$SIGNAME = (Packet[0] & S.getMask<uint8_t>());"
  elif [ $WIDTH -lt 33 ]; then
    # less than 32 bits
    # Will need to decide byte order between packets <-> verilator
    J=0
    BITJ=0
    echo "// less than 32 bits"
    echo "SignalHelper S($WIDTH);"
    echo "uint32_t tmp = 0;"
    until ((BITJ >= $WIDTH))
    do
      echo "tmp = (tmp<<8) + (((uint32_t)Packet[$J]) & 255);"
      J=$((J+1))
      BITJ=$((J*8))
    done
    echo "T->$SIGNAME = (tmp & S.getMask<uint32_t>());"
  elif [ $WIDTH -lt 65 ]; then
    # less than 64 bits
    echo "// less than 64 bits"
    J=0
    BITJ=0
    echo "SignalHelper S($WIDTH);"
    echo "uint64_t tmp = 0;"
    until ((BITJ >= $WIDTH))
    do
      echo "tmp = (tmp<<8) + (((uint64_t)Packet[$J]) & 255);"
      J=$((J+1))
      BITJ=$((J*8))
    done
    echo "T->$SIGNAME = (tmp & S.getMask<uint64_t>());"
  else
    # > 64 bits
    echo "// greater than 64 bits"
  fi
}

for IN in $INPUTS;do
  NOPAREN=`sed 's/.*(\(.*\))/\1/' <<< $IN`
  SIGNAME=`echo $NOPAREN | sed "s/,/ /g" | awk '{print $1}' | sed "s/&//g"`
  ENDBIT=`echo $NOPAREN | sed "s/,/ /g" | awk '{print $2}'`
  STARTBIT=`echo $NOPAREN | sed "s/,/ /g" | awk '{print $3}' | sed "s/;//g"`
  ENDBIT=$(($ENDBIT + 1))
  WIDTH=$(($ENDBIT - $STARTBIT))

  echo "void VerilatorSST$Device::DirectWrite${SIGNAME}(VTop *T,std::string Port,std::vector<uint8_t> Packet){"
  build_write $SIGNAME $WIDTH
  echo "}"
  echo "void VerilatorSST$Device::DirectWriteAtTick${SIGNAME}(VTop *T,std::string Port,std::vector<uint8_t> Packet,uint64_t Tick){"
  echo "}"
  echo "std::vector<uint8_t> VerilatorSST$Device::DirectRead${SIGNAME}(std::string Port){"
  echo "std::vector<uint8_t> d;"
  echo "return d;"
  echo "}"
done;

#-- Generate all the output signals
for OUT in $OUTPUTS;do
  NOPAREN=`sed 's/.*(\(.*\))/\1/' <<< $OUT`
  SIGNAME=`echo $NOPAREN | sed "s/,/ /g" | awk '{print $1}' | sed "s/&//g"`
  echo "void VerilatorSST$Device::DirectWrite${SIGNAME}(VTop *T,std::string Port,std::vector<uint8_t> Packet){"
  echo "}"
  echo "void VerilatorSST$Device::DirectWriteAtTick${SIGNAME}(VTop *T,std::string Port,std::vector<uint8_t> Packet,uint64_t Tick){"
  echo "}"
  echo "std::vector<uint8_t> VerilatorSST$Device::DirectRead${SIGNAME}(std::string Port){"
  echo "std::vector<uint8_t> d;"
  echo "return d;"
  echo "}"
done;


# -- EOF
