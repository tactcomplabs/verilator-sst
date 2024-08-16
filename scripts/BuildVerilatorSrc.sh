#!/bin/bash
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details

BUILDDIR=$1
SOURCEDIR=$2
TOP=$3
SRC=$4
ENABLE_INOUT_HANDLING=$5

ARGS=""
if [[ "$ENABLE_INOUT_HANDLING" == "ON" ]]; then
    ARGS="$ARGS --pins-inout-enables"
fi

verilator --cc --vpi --public-flat-rw $ARGS -CFLAGS "-fPIC -std=c++17" --Mdir $BUILDDIR -y $SOURCEDIR --prefix VTop --top-module $TOP $SRC
cd $BUILDDIR
make -f VTop.mk

# EOF
