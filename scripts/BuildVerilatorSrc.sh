#!/bin/bash
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details

set -e
BUILDDIR=$1
SOURCEDIR=$2
TOP=$3
SRC=$4
OPTIONS=$5
ENABLE_INOUT_HANDLING=$6

if [[ "$ENABLE_INOUT_HANDLING" == "ON" ]]; then
  echo "OPTIONS = $OPTIONS"
  OPTIONS="$OPTIONS --pins-inout-enables"
  echo "OPTIONS = $OPTIONS"
fi

verilator --cc --vpi --public-flat-rw -CFLAGS "-fPIC -std=c++17" --Mdir $BUILDDIR -y $SOURCEDIR --prefix VTop --top-module $TOP $SRC $OPTIONS
cd $BUILDDIR
make -f VTop.mk

# EOF
