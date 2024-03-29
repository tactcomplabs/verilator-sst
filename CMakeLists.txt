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
option(VERILOG_SOURCE "../verilog" CACHE STRING "Path to the verilog source tree")
if(EXISTS "${VERILOG_SOURCE}")
  message(STATUS "Found verilog source at ${VERILOG_SOURCE}")
else()
  message(FATAL "Could not find verilog source at ${VERILOG_SOURCE}")
endif()

option(VERILOG_DEVICE "Device" CACHE STRING "Name of the verilog device being simulated")

option(VERILOG_TOP "Top" CACHE STRING "Name of the verilog Top module")

option(VERILOG_TOP_SOURCES "top.v" CACHE STRING "List of verilog top source files")

option(VERILATOR_OPTIONS "" CACHE STRING "Additional verilator compilation options")

#------------------------------------------------------------------
# VERILATOR SETUP
#------------------------------------------------------------------
find_package(verilator HINTS $ENV{VERILATOR_ROOT} ${VERILATOR_ROOT})
if (NOT verilator_FOUND)
  message(FATAL_ERROR "Verilator was not found. Either install it, or set the VERILATOR_ROOT environment variable")
endif()

#------------------------------------------------------------------
# ADD THE SUBDIRECTORIES
#------------------------------------------------------------------
add_subdirectory(verilater-sst-element)

# EOF
