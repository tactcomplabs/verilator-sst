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

option(VERILATOR_INCLUDE "" "Sets the verilator include paht")

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
set(CXXFLAGS "${SST_CXXFLAGS}  -fno-stack-protector") #TODO: ask john about this, appears to be overwritten by line 111
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

set(VERILATORSST_EXTERNAL_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/verilator-sst-element")

#------------------------------------------------------------------
# ADD THE SUBDIRECTORIES
#------------------------------------------------------------------
add_subdirectory(verilator-sst-element)

option(VERILATORSST_ENABLE_TESTING "Enable Testing" ON)
if(VERILATORSST_ENABLE_TESTING)
  add_subdirectory(tests)
endif()


# EOF
