#!/bin/bash
# CheckInoutPorts.sh
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details

Top=$1
INOUTS=$(grep "VL_INOUT" $Top)

if [[ -z $INOUTS ]]; then
  exit 0
fi

exit 1
