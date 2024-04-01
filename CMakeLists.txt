# VerilatorSST Top-Level CMakeLists.txt
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details

# Prevent in-source builds
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "DO NOT BUILD in-tree.")
endif()

# Minimum required version of CMake and project information
cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0074 NEW)

project(verilatorSST CXX)

#------------------------------------------------------------------
# PROJECT ARGUMENTS
#------------------------------------------------------------------
option(VERILOG_SOURCE "../verilog" "Path to the verilog source tree")
if(EXISTS "${VERILOG_SOURCE}")
  message(STATUS "Found verilog source at ${VERILOG_SOURCE}")
else()
  message(FATAL "Could not find verilog source at ${VERILOG_SOURCE}")
endif()

option(VERILOG_DEVICE "Device" "Name of the verilog device being simulated")

option(VERILOG_TOP "Top" "Name of the verilog Top module")

option(VERILOG_TOP_SOURCES "top.v" "List of verilog top source files")

option(VERILATOR_OPTIONS "" "Additional verilator compilation options")

#------------------------------------------------------------------
# VERILATOR SETUP
#------------------------------------------------------------------
find_package(verilator HINTS $ENV{VERILATOR_ROOT} ${VERILATOR_ROOT})
if (NOT verilator_FOUND)
  message(FATAL_ERROR "Verilator was not found. Either install it, or set the VERILATOR_ROOT environment variable")
endif()

#------------------------------------------------------------------
# SST SETUP
#------------------------------------------------------------------
# SST Environment Derivation
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
set(CXXFLAGS "${SST_CXXFLAGS}  -fno-stack-protector")
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

#------------------------------------------------------------------
# ADD THE SUBDIRECTORIES
#------------------------------------------------------------------
add_subdirectory(verilator-sst-element)

# EOF
