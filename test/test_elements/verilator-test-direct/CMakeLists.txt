# tests/test_elements/verilator-test-direct/CMakeLists.txt
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

set(VTDSrcs
VerilatorTestDirect.cpp
VerilatorTestDirect.h
SST.h
)

add_library(verilatortestdirect SHARED ${VTDSrcs})
target_include_directories(verilatortestdirect
                PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                        ${VERILATORSST_EXTERNAL_INCLUDE}
                        ${VERILATORSST_EXTERNAL_INCLUDE}/${VERILOG_DEVICE}
                PUBLIC ${SST_INSTALL_DIR}/include
                       ${VERILATOR_INCLUDE}
                       ${VERILATOR_INCLUDE}/vltstd)


install(TARGETS verilatortestdirect DESTINATION ${CMAKE_SOURCE_DIR}/install)
install(CODE "execute_process(COMMAND sst-register verilatortestdirect verilatortestdirect_LIBDIR=${CMAKE_SOURCE_DIR}/install)")
