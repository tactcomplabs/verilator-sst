# examples/accum-driver-direct CMakeLists.txt
# Copyright (C) 2017-2026 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details

# NOTE: like accum-driver, this only needs verilatorSSTAPI.h (for
# VerilatorSSTBase); it never includes or links anything generated from verilog.
# Because it calls VerilatorSSTBase, which is defined in the generated
# subcomponent library, symbols are resolved at load time (SST's
# -undefined dynamic_lookup), same as the test elements.
add_library(accumdriverdirect SHARED AccumDriverDirect.cpp AccumDriverDirect.h)
set_property(TARGET accumdriverdirect PROPERTY CXX_STANDARD 17)
target_include_directories(accumdriverdirect
                PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                        ${VERILATORSST_EXTERNAL_INCLUDE}
                PUBLIC ${SST_INSTALL_DIR}/include
                       ${VERILATOR_INCLUDE}
                       ${VERILATOR_INCLUDE}/vltstd)

install(TARGETS accumdriverdirect DESTINATION ${CMAKE_SOURCE_DIR}/install)
install(CODE "execute_process(COMMAND sst-register accumdriverdirect accumdriverdirect_LIBDIR=${CMAKE_SOURCE_DIR}/install)")

add_test(NAME AccumDriverDirectExample
  COMMAND sst ${CMAKE_CURRENT_SOURCE_DIR}/accum-driver-direct.py -- -t 20)
set_tests_properties(AccumDriverDirectExample PROPERTIES
  PASS_REGULAR_EXPRESSION "AccumDriverDirect: PASSED"
  FAIL_REGULAR_EXPRESSION "AccumDriverDirect: FAILED")

# EOF
