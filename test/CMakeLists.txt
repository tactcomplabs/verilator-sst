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
  "Counter"
  "${CMAKE_CURRENT_SOURCE_DIR}/counter/Counter.v"
  "${CMAKE_CURRENT_SOURCE_DIR}/counter"
  ""
  "Counter"
  "Direct"
  "clk"
)

generate_verilator_component(
  "Counter"
  "${CMAKE_CURRENT_SOURCE_DIR}/counter/Counter.v"
  "${CMAKE_CURRENT_SOURCE_DIR}/counter"
  ""
  "Counter"
  "Links"
  "clk"
)

generate_verilator_component(
  "Accum"
  "${CMAKE_CURRENT_SOURCE_DIR}/accum/Accum.sv"
  "${CMAKE_CURRENT_SOURCE_DIR}/accum"
  ""
  "Accum"
  "Direct"
  "clk"
)

generate_verilator_component(
  "Accum"
  "${CMAKE_CURRENT_SOURCE_DIR}/accum/Accum.sv"
  "${CMAKE_CURRENT_SOURCE_DIR}/accum"
  ""
  "Accum"
  "Links"
  "clk"
)

generate_verilator_component(
  "Accum1D"
  "${CMAKE_CURRENT_SOURCE_DIR}/accum1D/Accum1D.sv"
  "${CMAKE_CURRENT_SOURCE_DIR}/accum1D"
  ""
  "Accum1D"
  "Direct"
  "clk"
)

generate_verilator_component(
  "Accum1D"
  "${CMAKE_CURRENT_SOURCE_DIR}/accum1D/Accum1D.sv"
  "${CMAKE_CURRENT_SOURCE_DIR}/accum1D"
  ""
  "Accum1D"
  "Links"
  "clk"
)

generate_verilator_component(
  "Scratchpad"
  "${CMAKE_CURRENT_SOURCE_DIR}/scratchpad/Scratchpad.sv"
  "${CMAKE_CURRENT_SOURCE_DIR}/scratchpad"
  ""
  "Scratchpad"
  "Direct"
  "clk"
)

generate_verilator_component(
  "Scratchpad"
  "${CMAKE_CURRENT_SOURCE_DIR}/scratchpad/Scratchpad.sv"
  "${CMAKE_CURRENT_SOURCE_DIR}/scratchpad"
  ""
  "Scratchpad"
  "Links"
  "clk"
)

generate_verilator_component(
  "UART"
  "${CMAKE_CURRENT_SOURCE_DIR}/uart_mem/*.sv"
  "${CMAKE_CURRENT_SOURCE_DIR}/uart_mem"
  ""
  "UART"
  "Direct"
  "clk"
)

generate_verilator_component(
  "UART"
  "${CMAKE_CURRENT_SOURCE_DIR}/uart_mem/*.sv"
  "${CMAKE_CURRENT_SOURCE_DIR}/uart_mem"
  ""
  "UART"
  "Links"
  "clk"
)

if( ENABLE_INOUT_HANDLING )
  generate_verilator_component(
    "Pin"
    "${CMAKE_CURRENT_SOURCE_DIR}/pin/Pin.sv"
    "${CMAKE_CURRENT_SOURCE_DIR}/pin"
    ""
    "Pin"
    "Links"
    "clk"
  )
  generate_verilator_component(
    "Pin"
    "${CMAKE_CURRENT_SOURCE_DIR}/pin/*.sv"
    "${CMAKE_CURRENT_SOURCE_DIR}/pin"
    ""
    "Pin"
    "Direct"
    "clk"
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
function(add_verilatorsst_test SUBDIRECTORY CYCLE_LIMIT)

  # Set the VTOP variable based on the subdirectory name passed as an argument
  set(VTOP ${SUBDIRECTORY})

  add_test(NAME VerilatorTestLink_${SUBDIRECTORY}
    COMMAND sst ${CMAKE_CURRENT_SOURCE_DIR}/test_elements/verilator-test-component.py -- -m ${VTOP} -i "links" -c ${CYCLE_LIMIT})
  add_test(NAME VerilatorTestDirect_${SUBDIRECTORY}
                  COMMAND sst ${CMAKE_CURRENT_SOURCE_DIR}/test_elements/verilator-test-component.py -- -m ${VTOP} -i "direct" -c ${CYCLE_LIMIT})
endfunction()

add_verilatorsst_test(Counter 50)
add_verilatorsst_test(Accum 50)
add_verilatorsst_test(Accum1D 50)
add_verilatorsst_test(Scratchpad 50)
add_verilatorsst_test(UART 512)
if(ENABLE_INOUT_HANDLING)
add_verilatorsst_test(Pin 50)
endif()
# EOF
