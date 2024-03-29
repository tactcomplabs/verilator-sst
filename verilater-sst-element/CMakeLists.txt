# verilater-sst-element CMakeLists.txt
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

# -----------------------------------------------------------------
# Generate the signals from the Verilator top
# -----------------------------------------------------------------
execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/BuildPortDef.sh
                OUTPUT_VARIABLE VERILATOR_SST_PORT_DEF
                OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/BuildPortEntry.sh
                OUTPUT_VARIABLE VERILATOR_SST_PORT_ENTRY
                OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/BuildPortMap.sh
                OUTPUT_VARIABLE VERILATOR_SST_PORT_MAP
                OUTPUT_STRIP_TRAILING_WHITESPACE)


# -----------------------------------------------------------------
# Configure the verilatorSST header
# -----------------------------------------------------------------
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/verilatorSST.h.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/verilatorSST.h"
)

# -----------------------------------------------------------------
# Build the entire source
# -----------------------------------------------------------------
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
#add_library(foo SHARED ${FooSrcs})
#target_include_directories(foo PRIVATE ${REVCPU_INCLUDE_PATH}
#                           PUBLIC ${SST_INSTALL_DIR}/include)

# -----------------------------------------------------------------
# Install the source
# -----------------------------------------------------------------
#install(TARGETS foo DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
#install(CODE "execute_process(COMMAND sst-register revcpu revcpu_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR})")
#if( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
#  install(CODE "execute_process(COMMAND_ERROR_IS_FATAL ANY COMMAND_ECHO STDERR COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/test_undefined_symbols.sh ${CMAKE_CURRENT_SOURCE_DIR}/librevcpu.dylib)")
#else()
#  install(CODE "execute_process(COMMAND_ERROR_IS_FATAL ANY COMMAND_ECHO STDERR COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/test_undefined_symbols.sh ${CMAKE_CURRENT_SOURCE_DIR}/librevcpu.so)")
#endif()


# -- EOF
