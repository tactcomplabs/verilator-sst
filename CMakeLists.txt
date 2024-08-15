# VerilatorSST Top-Level CMakeLists.txt
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details

# Prevent in-source builds
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "DO NOT BUILD IN-TREE!")
endif()

# Minimum required version of CMake and project information
cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0074 NEW)

project(verilatorSST CXX)

#------------------------------------------------------------------
# PROJECT ARGUMENTS
#------------------------------------------------------------------
option(VERILOG_SOURCE "../verilog" "Path to the verilog source tree")

option(VERILOG_DEVICE "Device" "Name of the verilog device being simulated")

option(VERILOG_TOP "Top" "Name of the verilog Top module")

option(VERILOG_TOP_SOURCES "top.v" "List of verilog top source files")

option(VERILATOR_OPTIONS "" "Additional verilator compilation options")

option(VERILATOR_INCLUDE "" "Sets the verilator include path")

option(VERILATORSST_ENABLE_TESTING "Enable Testing" ON)

option(ENABLE_CLK_HANDLING "Allows subcomponent to handle the clock port automatically" OFF)

option(ENABLE_LINK_HANDLING "Automatically adds SST link definitions and configurations" ON)

option(ENABLE_INOUT_HANDLING "Enables write/read access to verilog top module inout ports, requires Verilator 5.026+" OFF)

set(CLOCK_PORT_NAME "clk" CACHE STRING "Name of the top-level module's clock port") #Defaults to "clk"

#------------------------------------------------------------------
# VERILATOR SETUP
#------------------------------------------------------------------
find_package(verilator HINTS $ENV{VERILATOR_ROOT} ${VERILATOR_ROOT})
if (NOT verilator_FOUND)
  message(FATAL_ERROR
          "Verilator was not found. Either install it, or set the VERILATOR_ROOT environment variable")
endif()
if( NOT VERILATOR_INCLUDE)
  set(VERILATOR_INCLUDE "${VERILATOR_ROOT}/include")
endif()

find_program(VERILATOR_BIN verilator)
if(NOT VERILATOR_BIN)
  message(FATAL_ERROR "No verilator binary found in path")
endif()

execute_process(COMMAND verilator --version
                OUTPUT_VARIABLE VERILATOR_VERSION_STRING
                OUTPUT_STRIP_TRAILING_WHITESPACE
)
string(REPLACE " " ";" VERILATOR_VERSION_LIST ${VERILATOR_VERSION_STRING})
list(GET VERILATOR_VERSION_LIST 1 VERILATOR_VERSION)
string(REPLACE "." ";" VERILATOR_VERSION_SUBLIST ${VERILATOR_VERSION})
list(GET VERILATOR_VERSION_SUBLIST 0 VERILATOR_VERSION_MAJOR)
list(GET VERILATOR_VERSION_SUBLIST 1 VERILATOR_VERSION_MINOR_TMP)
string(REGEX REPLACE "^0+" " " VERILATOR_VERSION_MINOR ${VERILATOR_VERSION_MINOR_TMP})

if(${VERILATOR_VERSION_MAJOR} GREATER_EQUAL 5)
  if( ${VERILATOR_VERSION_MINOR} GREATER_EQUAL 22)
    if( ENABLE_INOUT_HANDLING )
      if (${VERILATOR_VERSION_MINOR} LESS 26)
        message(FATAL_ERROR "[VERILATOR] ENABLE_INOUT_HANDLING requires Verilator version 5.026+, found ${VERILATOR_VERSION}")
      endif()
    endif()
    message(STATUS "[VERILATOR] Found verilator version ${VERILATOR_VERSION}")
  else()
    message(STATUS "[VERILATOR] Found verilator major version ${VERILATOR_VERSION_MAJOR}")
    message(FATAL_ERROR
      "[VERILATOR] Verilator version is insufficient, use 5.022+: ${VERILATOR_VERSION}")
  endif()
else()
  message(STATUS "[VERILATOR] Found verilator major version ${VERILATOR_VERSION_MAJOR}")
  message(FATAL_ERROR
    "[VERILATOR] Verilator version is insufficient, use 5.022+: ${VERILATOR_VERSION}")
endif()

get_filename_component(VERILATOR_PATH ${VERILATOR_BIN} DIRECTORY)
message(STATUS "[VERILATOR] VERILATOR_INCLUDE set to ${VERILATOR_INCLUDE}")


#------------------------------------------------------------------
# SST SETUP
#------------------------------------------------------------------
find_program(SST sst)
find_program(SST_CONFIG sst-config)
if(NOT SST OR NOT SST_CONFIG)
  message(FATAL_ERROR "No SST binary or sst-config binary found in path")
endif()
execute_process(COMMAND sst-config --prefix
                OUTPUT_VARIABLE SST_INSTALL_DIR
                OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(COMMAND sst-config --CXX
                OUTPUT_VARIABLE CXX
                OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(COMMAND sst-config --ELEMENT_CXXFLAGS
                OUTPUT_VARIABLE SST_CXXFLAGS
                OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(COMMAND sst-config --ELEMENT_LDFLAGS
                OUTPUT_VARIABLE SST_LDFLAGS
                OUTPUT_STRIP_TRAILING_WHITESPACE
)

message(STATUS "[SST] SST_INSTALL_DIR=${SST_INSTALL_DIR}")
message(STATUS "[SST] SST CXX=${CXX}")
message(STATUS "[SST] SST_CXXFLAGS=${SST_CXXFLAGS}")
message(STATUS "[SST] SST_LDFLAGS=${SST_LDFLAGS}")

set(CXXFLAGS "${SST_CXXFLAGS}")
set(LDFLAGS "${SST_LDFLAGS}")
separate_arguments(SST_CXXFLAGS_LIST UNIX_COMMAND ${CXXFLAGS})
separate_arguments(SST_LDFLAGS_LIST UNIX_COMMAND ${LDFLAGS})
add_compile_options("${SST_CXXFLAGS_LIST}")
add_link_options("${SST_LDFLAGS_LIST}")

set(VERILATORSST_EXTERNAL_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/verilator-sst-element")
set(VERILATORSST_SCRIPTS "${CMAKE_CURRENT_SOURCE_DIR}/scripts")

#------------------------------------------------------------------
# ADD THE SUBDIRECTORIES
#------------------------------------------------------------------
add_subdirectory(verilator-sst-element)
#
# option to enable testing (default is ON)
option(ENABLE_TESTING "Enable testing" ON)
# Also allows users to pass -DDISABLE_TESTING
if(DISABLE_TESTING)
  set(ENABLE_TESTING OFF)
endif()

# Check if testing is enabled and add the tests subdirectory if it is
if(ENABLE_TESTING)
  enable_testing()
  add_subdirectory(test)
endif()

# EOF
