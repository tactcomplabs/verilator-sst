cmake_minimum_required(VERSION 3.13)

project(basicverilogcounter)
find_package(verilatorsst HINTS ${CMAKE_CURRENT_SOURCE_DIR}/../..)
add_library(basicverilogcounter SHARED "")
verilatorsstf(basicverilogcounter
    DEBUG
    VSRC_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/vsrc
    CSRC_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/csrc
    TOP_MODULE  ${CMAKE_CURRENT_SOURCE_DIR}/vsrc/Counter.v
    VERILATOR_ARGS "-DSTOP_WIDTH=16")
