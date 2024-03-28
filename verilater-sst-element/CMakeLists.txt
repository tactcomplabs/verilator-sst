# verilater-sst-element CMakeLists.txt
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/BuildPortMapping.sh
                OUTPUT_VARIABLE VERILATOR_SST_PORT_DEF
                OUTPUT_STRIP_TRAILING_WHITESPACE)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/verilatorSST.h.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/verilatorSST.h"
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# -- EOF
