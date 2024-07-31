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
  ${CMAKE_CURRENT_SOURCE_DIR}/accum1D
  "Accum1D"
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

# ------------------------------------------------ #
# Tests
# ------------------------------------------------ #
function(add_verilatorsst_test SUBDIRECTORY)
  # Set the VTOP variable based on the subdirectory name passed as an argument
  set(VTOP ${SUBDIRECTORY})

  add_test(NAME VerilatorTestLink_${SUBDIRECTORY}
                  COMMAND sst ${CMAKE_CURRENT_SOURCE_DIR}/test_elements/verilator-test-component.py -- -m ${VTOP} -i "links")
  add_test(NAME VerilatorTestDirect_${SUBDIRECTORY}
                  COMMAND sst ${CMAKE_CURRENT_SOURCE_DIR}/test_elements/verilator-test-component.py -- -m ${VTOP} -i "direct")
endfunction()

add_verilatorsst_test(Counter)
add_verilatorsst_test(Accum)
add_verilatorsst_test(Accum1D)
add_verilatorsst_test(Scratchpad)
add_verilatorsst_test(UART)
# EOF
