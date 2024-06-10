option(VTOP "Top module header path")
option(VERILOG_TOP "Top module source path")

if(NOT VTOP)
    message(FATAL_ERROR "VTOP option required")
endif()
if(NOT VERILOG_TOP)
    message(FATAL_ERROR "VERILOG_TOP option required")
endif()

set(VERILOG_DEVICE ${VERILOG_TOP})

message(STATUS "Building port definitions...")
execute_process(COMMAND ${SCRIPTS_DR}/BuildPortDef.sh ${VTOP}
                OUTPUT_VARIABLE VERILATOR_SST_PORT_DEF
                OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "Building port entries...")
execute_process(COMMAND ${SCRIPTS_DR}/BuildPortEntry.sh ${VTOP} ${VERILOG_DEVICE}
                OUTPUT_VARIABLE VERILATOR_SST_PORT_ENTRY
                OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "Building port map...")
execute_process(COMMAND ${SCRIPTS_DR}/BuildPortMap.sh ${VTOP}
                OUTPUT_VARIABLE VERILATOR_SST_PORT_MAP
                OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "Building port handlers...")
execute_process(COMMAND ${SCRIPTS_DR}/BuildPortHandlers.sh ${VTOP}
                OUTPUT_VARIABLE VERILATOR_SST_PORT_HANDLERS
                OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "Building port IO...")
execute_process(COMMAND ${SCRIPTS_DR}/BuildPortIO.sh ${VTOP}
                OUTPUT_VARIABLE VERILATOR_SST_PORT_IO_HANDLERS
                OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "Building port IO implementations...")
execute_process(COMMAND ${SCRIPTS_DR}/BuildPortIOImpls.sh ${VTOP} ${VERILOG_DEVICE}
                OUTPUT_VARIABLE VERILATOR_SST_PORT_IO_IMPLS
                OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "Building port handler implementations...")
execute_process(COMMAND ${SCRIPTS_DR}/BuildPortHandlerImpls.sh ${VTOP} ${VERILOG_DEVICE}
                OUTPUT_VARIABLE VERILATOR_SST_PORT_HANDLER_IMPLS
                OUTPUT_STRIP_TRAILING_WHITESPACE)

# -----------------------------------------------------------------
# Configure the verilatorSST files
# -----------------------------------------------------------------
configure_file(
  "${SOURCE_DIR}/verilatorSSTSubcomponent.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/verilatorSSTSubcomponent.h"
)

configure_file(
  "${SOURCE_DIR}/verilatorSSTSubcomponent.cpp.in"
  "${CMAKE_CURRENT_BINARY_DIR}/verilatorSSTSubcomponent.cpp"
)

# -----------------------------------------------------------------
# Format the source files
# -----------------------------------------------------------------
find_program(CLANG_FORMAT "clang-format")
if( CLANG_FORMAT )
  message(STATUS "[CLANG-FORMAT] Executing clang-format on generated source files")
  execute_process(COMMAND ${CLANG_FORMAT} -i --assume-filename=${SOURCE_DIR}/.clang-format ${CMAKE_CURRENT_BINARY_DIR}/verilatorSSTSubcomponent.cpp)
  execute_process(COMMAND ${CLANG_FORMAT} -i --assume-filename=${SOURCE_DIR}/.clang-format ${CMAKE_CURRENT_BINARY_DIR}/verilatorSSTSubcomponent.h)
endif()
