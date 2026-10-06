# examples/accum-driver CMakeLists.txt
# Copyright (C) 2017-2026 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details

# NOTE: the driver only needs verilatorSSTAPI.h (PortEvent). It does NOT link
# against or include anything generated from the verilog; it talks to the
# verilated model purely through SST links.
add_library(accumdriver SHARED AccumDriver.cpp AccumDriver.h)
set_property(TARGET accumdriver PROPERTY CXX_STANDARD 17)
target_include_directories(accumdriver
                PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                        ${VERILATORSST_EXTERNAL_INCLUDE}
                PUBLIC ${SST_INSTALL_DIR}/include
                       ${VERILATOR_INCLUDE}
                       ${VERILATOR_INCLUDE}/vltstd)

install(TARGETS accumdriver DESTINATION ${CMAKE_SOURCE_DIR}/install)
install(CODE "execute_process(COMMAND sst-register accumdriver accumdriver_LIBDIR=${CMAKE_SOURCE_DIR}/install)")

add_test(NAME AccumDriverExample
  COMMAND sst ${CMAKE_CURRENT_SOURCE_DIR}/accum-driver.py -- -t 20)
set_tests_properties(AccumDriverExample PROPERTIES
  PASS_REGULAR_EXPRESSION "AccumDriver: PASSED"
  FAIL_REGULAR_EXPRESSION "AccumDriver: FAILED")

# EOF
