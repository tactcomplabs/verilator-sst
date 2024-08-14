# tests/ CMakeLists.txt
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details
#
# ------------------------------------------------ #
# Build the test components
# ------------------------------------------------ #
generate_verilator_component(
  ${CMAKE_CURRENT_SOURCE_DIR}/counter
  "Counter"
  "${CMAKE_CURRENT_SOURCE_DIR}/counter/Counter.v"
  "$PWD/tests/counter/Counter.v"
  "Counter"
  "Direct"
)

generate_verilator_component(
  ${CMAKE_CURRENT_SOURCE_DIR}/counter
  "Counter"
  "${CMAKE_CURRENT_SOURCE_DIR}/counter/Counter.v"
  "$PWD/tests/counter/Counter.v"
  "Counter"
  "Links"
)

generate_verilator_component(
  ${CMAKE_CURRENT_SOURCE_DIR}/accum
  "Accum"
  "${CMAKE_CURRENT_SOURCE_DIR}/accum/Accum.sv"
  "$PWD/tests/accum/Accum.sv"
  "Accum"
  "Direct"
)

generate_verilator_component(
  ${CMAKE_CURRENT_SOURCE_DIR}/accum
  "Accum"
  "${CMAKE_CURRENT_SOURCE_DIR}/accum/Accum.sv"
  "$PWD/tests/accum/Accum.sv"
  "Accum"
  "Links"
)

generate_verilator_component(
  ${CMAKE_CURRENT_SOURCE_DIR}/accum1D
  "Accum1D"
  "${CMAKE_CURRENT_SOURCE_DIR}/accum1D/Accum1D.sv"
  "$PWD/tests/accum1D/Accum1D.sv"
  "Accum1D"
  "Direct"
)

generate_verilator_component(
  ${CMAKE_CURRENT_SOURCE_DIR}/accum1D
  "Accum1D"
  "${CMAKE_CURRENT_SOURCE_DIR}/accum1D/Accum1D.sv"
  "$PWD/tests/accum1D/Accum1D.sv"
  "Accum1D"
  "Links"
)

generate_verilator_component(
  ${CMAKE_CURRENT_SOURCE_DIR}/scratchpad
  "Scratchpad"
  "${CMAKE_CURRENT_SOURCE_DIR}/scratchpad/Scratchpad.sv"
  "$PWD/tests/scratchpad/*.sv"
  "Scratchpad"
  "Direct"
)

generate_verilator_component(
  ${CMAKE_CURRENT_SOURCE_DIR}/scratchpad
  "Scratchpad"
  "${CMAKE_CURRENT_SOURCE_DIR}/scratchpad/Scratchpad.sv"
  "$PWD/tests/scratchpad/*.sv"
  "Scratchpad"
  "Links"
)

generate_verilator_component(
  ${CMAKE_CURRENT_SOURCE_DIR}/uart_mem
  "UART"
  "${CMAKE_CURRENT_SOURCE_DIR}/uart_mem/*.sv"
  "$PWD/tests/uart_mem/UART.sv"
  "UART"
  "Direct"
)

generate_verilator_component(
  ${CMAKE_CURRENT_SOURCE_DIR}/uart_mem
  "UART"
  "${CMAKE_CURRENT_SOURCE_DIR}/uart_mem/*.sv"
  "$PWD/tests/uart_mem/UART.sv"
  "UART"
  "Links"
)

if( ENABLE_INOUT_HANDLING )
  generate_verilator_component(
    ${CMAKE_CURRENT_SOURCE_DIR}/pin
    "Pin"
    "${CMAKE_CURRENT_SOURCE_DIR}/pin/Pin.sv"
    "$PWD/tests/pin/Pin.sv"
    "Pin"
    "Links"
  )
  generate_verilator_component(
    ${CMAKE_CURRENT_SOURCE_DIR}/pin
    "Pin"
    "${CMAKE_CURRENT_SOURCE_DIR}/pin/*.sv"
    "$PWD/tests/pin/Pin.sv"
    "Pin"
    "Direct"
  )
endif()

# ---------------------------------------------------------------------- #
# Add the subdirectory containing the standalone elements used to test
# the generated ones
# ---------------------------------------------------------------------- #
add_subdirectory(test_elements)

# ---------------------------------------------------------------------- #
# Add the test commands to CTest
# ---------------------------------------------------------------------- #
# - One test will run the link based, SST interface
# - One test will run the direct interface which does not use SST::Link
#---------------------- ------------------------------------------------ #
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
if(ENABLE_INOUT_HANDLING)
add_verilatorsst_test(Pin)
endif()
# EOF
