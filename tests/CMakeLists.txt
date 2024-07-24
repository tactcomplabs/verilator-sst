# tests/ CMakeLists.txt
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details

generate_verilator_component(
  ${CMAKE_CURRENT_SOURCE_DIR}/counter
  "Counter"
  "${CMAKE_CURRENT_SOURCE_DIR}/counter/Counter.v"
  "$PWD/tests/counter/Counter.v"
  "Counter"
)
add_subdirectory(test_elements)

# EOF
