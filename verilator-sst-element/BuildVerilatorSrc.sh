#!/bin/bash
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details

TARGETDIR=$1
TOP=$2
SRC=$3

echo "Executing verilator: "
echo "    verilator --cc --vpi -Wall --Mdir $TARGETDIR --prefix VTop --top-module $TOP $SRC"
verilator --cc --vpi -Wall --Mdir $TARGETDIR --prefix VTop --top-module $TOP $SRC
cd $TARGETDIR
make -f VTop.mk

# EOF
