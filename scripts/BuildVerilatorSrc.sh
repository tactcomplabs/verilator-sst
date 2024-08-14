#!/bin/bash
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details

TARGETDIR=$1
TOP=$2
SRC=$3
ENABLE_INOUT_HANDLING=$4

ARGS=""
if [[ "$ENABLE_INOUT_HANDLING" == "ON" ]]; then
    ARGS="$ARGS --pins-inout-enables"
fi

verilator --cc --vpi --public-flat-rw $ARGS -CFLAGS "-fPIC -std=c++17" --Mdir $TARGETDIR --prefix VTop --top-module $TOP $SRC
cd $TARGETDIR
make -f VTop.mk

# EOF
