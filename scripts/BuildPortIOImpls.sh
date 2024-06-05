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
    echo "// less than 32 bits"
    REMBITS=$((WIDTH % 8))
    if ((REMBITS == 0)); then
      PADBYTE=0
    else
      PADBYTE=1
    fi
    BYTES=$((WIDTH / 8 + PADBYTE - 1))
    echo "SignalHelper S($WIDTH);"
    echo "uint32_t tmp = 0;"
    until ((BYTES < 0))
    do
      echo "tmp = (tmp<<8) + (((uint32_t)Packet[$BYTES]) & 255);"
      BYTES=$((BYTES-1))
    done
    echo "T->$SIGNAME = (tmp & S.getMask<uint32_t>());"
  elif [ $WIDTH -lt 65 ]; then
    # less than 64 bits
    echo "// less than 64 bits"
    REMBITS=$((WIDTH % 8))
    if ((REMBITS == 0)); then
      PADBYTE=0
    else
      PADBYTE=1
    fi
    BYTES=$((WIDTH / 8 + PADBYTE - 1))
    echo "SignalHelper S($WIDTH);"
    echo "uint64_t tmp = 0;"
    until ((BYTES < 0))
    do
      echo "tmp = (tmp<<8) + (((uint64_t)Packet[$BYTES]) & 255);"
      BYTES=$((BYTES-1))
    done
    echo "T->$SIGNAME = (tmp & S.getMask<uint64_t>());"
  else
    # > 64 bits
    # Verilator splits these into struct VlWide which is m_storage[T_Words] of EData (uint32_t). Can be indexed directly
    echo "// greater than 64 bits"
    J=0
    BITJ=0
    REMWIDTH=$((WIDTH % 32))
    WORDS=$((WIDTH / 32))
    REMBITS=$((REMWIDTH % 8))
    if ((REMWIDTH == 0)); then
      # Width is word-aligned; no partial word
      REMBYTES=0
    elif ((REMBITS == 0)); then
      # Width is byte-aligned but not word aligned; partial word
      REMBYTES=$((REMWIDTH / 8))
    else
      # Width is not aligned; partial word with padding
      REMBYTES=$((REMWIDTH / 8 + 1))
    fi
    # Generate word-wise for loop
    echo "for (int i=0; i<$WORDS; i++) {"
    echo "uint32_t tmp = 0;"
    J=3
    # Generate for loop body: same process as <32 bits example
    until ((J < 0))
    do
      echo "tmp = (tmp<<8) + (((uint32_t)Packet[i*4+$J]) & 255);"
      J=$((J-1))
    done
    # Full words don't need masking
    echo "T->$SIGNAME[i] = tmp;"
    echo "}"
    if ((REMWIDTH != 0)); then
      # Case where there is a partial word
      echo "SignalHelper S($REMWIDTH);"
      echo "uint32_t tmp = 0;"
      J=$((REMBYTES - 1))
      until ((J < 0))
      do
        INDEX=$((WORDS * 4 + J))
        echo "tmp = (tmp<<8) + (((uint32_t)Packet[$INDEX]) & 255);"
        J=$((J-1))
      done
      echo "T->$SIGNAME[$WORDS] = (tmp & S.getMask<uint32_t>());"
    fi
    # TODO: may have to change order of word assignment
  fi
}

build_read () {
  SIGNAME=$1
  WIDTH=$2
  #Width aligned to an upper byte boundary
  REMBITS=$((WIDTH % 8))
  ALIGWIDTH=$((WIDTH + (8 - REMBITS)))
  if ((REMBITS == 0)); then
    ALIGWIDTH=$WIDTH
  fi

  if [ $WIDTH -lt 9 ]; then
    # less than a 8 bits
    echo "// less than 8 bit"
    echo "d.push_back(T->$SIGNAME);"
  elif [ $WIDTH -lt 33 ]; then
    echo "// less than 32 bit"
    echo "uint8_t tmp = 0;"
    LOOPI=0
    until ((LOOPI == ALIGWIDTH))
    do
      echo "tmp = (T->$SIGNAME >> $LOOPI) & 255;"
      echo "d.push_back(tmp);"
      LOOPI=$((LOOPI + 8))
    done
  elif [ $WIDTH -lt 65 ]; then
    echo "// less than 64 bit"
    echo "uint8_t tmp = 0;"
    LOOPI=0
    until ((LOOPI == ALIGWIDTH))
    do
      echo "tmp = (T->$SIGNAME >> $LOOPI) & 255;"
      echo "d.push_back(tmp);"
      LOOPI=$((LOOPI + 8))
    done
  else
    echo "// wider than 64 bits"
    REMWIDTH=$((WIDTH % 32))
    WORDS=$((WIDTH / 32 + 1))
    if ((REMWIDTH == 0)); then
      # Case where there is a partial word
      WORDS=$((WIDTH / 32))
    fi
    echo "uint8_t tmp = 0;"
    echo "for (int i=0; i<$WORDS; i++) {"
    LOOPI=0
    # Read out full words
    until ((LOOPI == 4))
    do
      SHIFTBIT=$((LOOPI * 8))
      echo "tmp = (T->$SIGNAME[i] >> $SHIFTBIT) & 255;"
      echo "d.push_back(tmp);"
      LOOPI=$((LOOPI + 1))
    done
    echo "}"
    if ((REMBITS != 0)); then
      # Read out partial word
      PARTALIGWIDTH=$((ALIGWIDTH - (WORDS-1) * 32))
      LOOPI=0
      until ((LOOPI == PARTALIGWIDTH))
      do
        echo "tmp = (T->$SIGNAME[$WORDS] >> $LOOPI) & 255;"
        echo "d.push_back(tmp);"
        LOOPI=$((LOOPI + 8))
      done
    fi
  fi
}

for IN in $INPUTS;do
  NOPAREN=`sed 's/.*(\(.*\))/\1/' <<< $IN`
  NOPAREN2=`echo $NOPAREN | sed 's/)//'`
  DEPTH=`echo $NOPAREN2 | sed 's/[][]/ /g' | awk '{print $2}'`
  if [ -z "$DEPTH" ]; then
    DEPTH=1
  fi
  REMDEPTH=`echo $NOPAREN2 | sed 's/\[[0-9]*\]//'`
  SIGNAME=`echo $REMDEPTH | sed "s/,/ /g" | awk '{print $1}' | sed "s/&//g"`
  ENDBIT=`echo $REMDEPTH | sed "s/,/ /g" | awk '{print $2}'`
  STARTBIT=`echo $REMDEPTH | sed "s/,/ /g" | awk '{print $3}' | sed "s/;//g"`
  ENDBIT=$(($ENDBIT + 1))
  WIDTH=$(($ENDBIT - $STARTBIT))

  echo "void VerilatorSST$Device::DirectWrite${SIGNAME}(VTop *T,std::vector<uint8_t> Packet){"
  build_write $SIGNAME $WIDTH
  echo "}"
  echo "std::vector<uint8_t> VerilatorSST$Device::DirectRead${SIGNAME}(VTop *T){"
  echo "std::vector<uint8_t> d;"
  echo "return d;"
  echo "}"
done;

#-- Generate all the output signals
for OUT in $OUTPUTS;do
  NOPAREN=`sed 's/.*(\(.*\))/\1/' <<< $OUT`
  NOPAREN2=`echo $NOPAREN | sed 's/)//'`
  DEPTH=`echo $NOPAREN2 | sed 's/[][]/ /g' | awk '{print $2}'`
  if [ -z "$DEPTH" ]; then
    DEPTH=1
  fi
  REMDEPTH=`echo $NOPAREN2 | sed 's/\[[0-9]*\]//'`
  SIGNAME=`echo $REMDEPTH | sed "s/,/ /g" | awk '{print $1}' | sed "s/&//g"`
  ENDBIT=`echo $REMDEPTH | sed "s/,/ /g" | awk '{print $2}'`
  STARTBIT=`echo $REMDEPTH | sed "s/,/ /g" | awk '{print $3}' | sed "s/;//g"`
  ENDBIT=$(($ENDBIT + 1))
  WIDTH=$(($ENDBIT - $STARTBIT))

  echo "void VerilatorSST$Device::DirectWrite${SIGNAME}(VTop *T,std::vector<uint8_t> Packet){"
  echo "}"
  echo "std::vector<uint8_t> VerilatorSST$Device::DirectRead${SIGNAME}(VTop *T){"
  echo "std::vector<uint8_t> d;"
  build_read $SIGNAME $WIDTH
  echo "return d;"
  echo "}"
done;


# -- EOF
