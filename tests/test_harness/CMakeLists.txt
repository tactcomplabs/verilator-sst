# tests/test_harness/CMakeLists.txt
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details

target_sources(${TESTS_TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/TestHarness.cpp)
target_include_directories(${TESTS_TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
