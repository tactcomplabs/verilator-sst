#!/bin/bash
# BuildVerilatorJSON.sh
#
# Copyright (C) 2017-2026 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details
#
# Runs Verilator's AST/JSON dump (--json-only) for the same top module and
# sources used by BuildVerilatorSrc.sh. This is a second, lightweight
# Verilator invocation (parse-only, no C++ codegen) whose sole purpose is to
# produce VTop.tree.json: an authoritative, version-proof description of
# every port's name/direction/width/array-depth that GeneratePortManifest.py
# turns into the manifest scripts/Build*.sh scripts consume in place of
# VTop.h. See GeneratePortManifest.py for why VTop.h's own C++ port
# declarations aren't parsed directly for this.

set -e
BUILDDIR=$1
SOURCEDIR=$2
TOP=$3
SRC=$4
OPTIONS=$5
ENABLE_INOUT_HANDLING=$6

if [[ "$ENABLE_INOUT_HANDLING" == "ON" ]]; then
  OPTIONS="$OPTIONS --pins-inout-enables"
fi

verilator --json-only $OPTIONS --Mdir $BUILDDIR -y $SOURCEDIR --prefix VTop --top-module $TOP $SRC

# EOF
