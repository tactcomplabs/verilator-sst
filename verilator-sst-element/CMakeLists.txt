# verilater-sst-element CMakeLists.txt
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#

# -----------------------------------------------------------------
# Generate the Verilator source
# Relevant CMake variables:
# - VERILOG_SOURCE : path to the verilog source
# - VERILOG_TOP : names of the top modules
# - VERILOG_TOP_SOURCES : list of the top module files
# - VERILATOR_OPTIONS : verilator compilation options
# - VERILOG_DEVICE : device name of the target verilog module
# -----------------------------------------------------------------

# -----------------------------------------------------------------
# Generate the signals from the Verilator top
# -----------------------------------------------------------------
set(VTOP "${CMAKE_CURRENT_SOURCE_DIR}/${VERILOG_DEVICE}/VTop.h")
execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/BuildVerilatorSrc.sh
                        ${VERILOG_DEVICE} ${VERILOG_TOP} ${VERILOG_TOP_SOURCES}
                OUTPUT_VARIABLE VERILATOR_OUT
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/BuildPortDef.sh ${VTOP}
                OUTPUT_VARIABLE VERILATOR_SST_PORT_DEF
                OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/BuildPortEntry.sh ${VTOP}
                OUTPUT_VARIABLE VERILATOR_SST_PORT_ENTRY
                OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/BuildPortMap.sh ${VTOP}
                OUTPUT_VARIABLE VERILATOR_SST_PORT_MAP
                OUTPUT_STRIP_TRAILING_WHITESPACE)


# -----------------------------------------------------------------
# Configure the verilatorSST files
# -----------------------------------------------------------------
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/verilatorSST.h.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/verilatorSST.h"
)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/verilatorSST.cpp.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/verilatorSST.cpp"
)


# -----------------------------------------------------------------
# Discover all the source files
# -----------------------------------------------------------------
MACRO(HEADER_DIRECTORIES return_list)
    FILE(GLOB_RECURSE new_list *.h)
    SET(dir_list "")
    FOREACH(file_path ${new_list})
        GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
        SET(dir_list ${dir_list} ${dir_path})
    ENDFOREACH()
    LIST(REMOVE_DUPLICATES dir_list)
    SET(${return_list} ${dir_list})
ENDMACRO()

HEADER_DIRECTORIES(header_dir_list)
list(LENGTH header_dir_list header_dir_list_count)
message(STATUS "[INFO] Found ${header_dir_list_count} header directories.")

MACRO(CPP_DIRECTORIES return_list)
    FILE(GLOB_RECURSE new_list *.cpp)
    SET(dir_list "")
    FOREACH(file_path ${new_list})
        GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
        SET(dir_list ${dir_list} ${dir_path})
    ENDFOREACH()
    LIST(REMOVE_DUPLICATES dir_list)
    SET(${return_list} ${dir_list})
ENDMACRO()

CPP_DIRECTORIES(cpp_dir_list)
list(LENGTH cpp_dir_list cpp_dir_list_count)
message(STATUS "[INFO] Found ${cpp_dir_list_count} cpp directories.")

file(GLOB_RECURSE VSRCS *.cpp)
file(GLOB_RECURSE VHDRS *.h)

# -----------------------------------------------------------------
# Build the entire source
# -----------------------------------------------------------------
set(targetName "verilatorsst${VERILOG_DEVICE}")
set(verilatorSSTSrcs
  verilatorSST.cpp
  verilatorSST.h
)

add_library(${targetName} SHARED ${verilatorSSTSrcs})
set_property(TARGET ${targetName} PROPERTY CXX_STANDARD 17)
target_include_directories(${targetName}
                          PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/${VERILOG_DEVICE}
                          PUBLIC ${SST_INSTALL_DIR}/include ${VERILATOR_INCLUDE})
target_link_libraries(${targetName}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${VERILOG_DEVICE}/VTop__ALL.a)

# -----------------------------------------------------------------
# Install the source
# -----------------------------------------------------------------
install(TARGETS ${targetName} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
install(CODE "execute_process(COMMAND sst-register ${targetName} ${targetName}_LIBDIR=${CMAKE_CURRENT_SOURCE_DIR})")

#if( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
#  install(CODE "execute_process(COMMAND_ERROR_IS_FATAL ANY COMMAND_ECHO STDERR COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/test_undefined_symbols.sh ${CMAKE_CURRENT_SOURCE_DIR}/librevcpu.dylib)")
#else()
#  install(CODE "execute_process(COMMAND_ERROR_IS_FATAL ANY COMMAND_ECHO STDERR COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/test_undefined_symbols.sh ${CMAKE_CURRENT_SOURCE_DIR}/librevcpu.so)")
#endif()


# -- EOF
