# examples/accum-offload CMakeLists.txt
# Copyright (C) 2017-2026 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details

# One library holds the client-facing API (AccumAccelAPI), its implementations
# (adapters over the Direct and Links builds of the verilated model, and a
# link client for a standalone AccumAccelComponent), and a stub CPU that uses
# any of them.
# Nothing here includes or links anything generated from the verilog;
# VerilatorSSTBase symbols resolve at load time, as in the other examples.
add_library(accumaccel SHARED
  AccumAccelAPI.h AccumAdapter.cpp AccumAdapter.h
  AccumEngine.cpp AccumEngine.h AccumEvents.h
  AccumLinksAdapter.cpp AccumLinksAdapter.h
  AccumAccelComponent.cpp AccumAccelComponent.h
  AccumLinkClient.cpp AccumLinkClient.h
  StubCpu.cpp StubCpu.h)
set_property(TARGET accumaccel PROPERTY CXX_STANDARD 17)
target_include_directories(accumaccel
                PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                        ${VERILATORSST_EXTERNAL_INCLUDE}
                PUBLIC ${SST_INSTALL_DIR}/include
                       ${VERILATOR_INCLUDE}
                       ${VERILATOR_INCLUDE}/vltstd)

install(TARGETS accumaccel DESTINATION ${CMAKE_SOURCE_DIR}/install)
install(CODE "execute_process(COMMAND sst-register accumaccel accumaccel_LIBDIR=${CMAKE_SOURCE_DIR}/install)")

add_test(NAME AccumOffloadExample
  COMMAND sst ${CMAKE_CURRENT_SOURCE_DIR}/accum-offload.py)
set_tests_properties(AccumOffloadExample PROPERTIES
  PASS_REGULAR_EXPRESSION "StubCpu: PASSED"
  FAIL_REGULAR_EXPRESSION "StubCpu: FAILED")

add_test(NAME AccumOffloadLinksExample
  COMMAND sst ${CMAKE_CURRENT_SOURCE_DIR}/accum-offload-links.py)
set_tests_properties(AccumOffloadLinksExample PROPERTIES
  PASS_REGULAR_EXPRESSION "StubCpu: PASSED"
  FAIL_REGULAR_EXPRESSION "StubCpu: FAILED")

add_test(NAME AccumOffloadComponentExample
  COMMAND sst ${CMAKE_CURRENT_SOURCE_DIR}/accum-offload-component.py)
add_test(NAME AccumOffloadComponentSharedExample
  COMMAND sst ${CMAKE_CURRENT_SOURCE_DIR}/accum-offload-component.py -- -c 3 -l 25ns)
set_tests_properties(AccumOffloadComponentExample
                     AccumOffloadComponentSharedExample PROPERTIES
  PASS_REGULAR_EXPRESSION "StubCpu: PASSED"
  FAIL_REGULAR_EXPRESSION "StubCpu: FAILED")

# EOF
