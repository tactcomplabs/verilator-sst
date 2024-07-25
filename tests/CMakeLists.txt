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

generate_verilator_component(
  ${CMAKE_CURRENT_SOURCE_DIR}/accum
  "Accum"
  "${CMAKE_CURRENT_SOURCE_DIR}/accum/Accum.sv"
  "$PWD/tests/accum/Accum.sv"
  "Accum"
)

generate_verilator_component(
  ${CMAKE_CURRENT_SOURCE_DIR}/accum
  "Accum"
  "${CMAKE_CURRENT_SOURCE_DIR}/accum1D/Accum1D.sv"
  "$PWD/tests/accum1D/Accum1D.sv"
  "Accum1D"
)

generate_verilator_component(
  ${CMAKE_CURRENT_SOURCE_DIR}/scratchpad
  "Scratchpad"
  "${CMAKE_CURRENT_SOURCE_DIR}/scratchpad/Scratchpad.sv"
  "$PWD/tests/scratchpad/*.sv"
  "Scratchpad"
)

generate_verilator_component(
  ${CMAKE_CURRENT_SOURCE_DIR}/uart_mem
  "UART"
  "${CMAKE_CURRENT_SOURCE_DIR}/uart_mem/*.sv"
  "$PWD/tests/uart_mem/UART.sv"
  "UART"
)
add_subdirectory(test_elements)

# EOF
