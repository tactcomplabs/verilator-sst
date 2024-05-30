cmake_minimum_required(VERSION 3.13)

project(bigverilogaccum)
find_package(verilatorsst HINTS ${CMAKE_CURRENT_SOURCE_DIR}/../..)
add_library(bigverilogaccum SHARED "")
verilatorsstf(bigverilogaccum
    VSRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vsrc
    CSRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/csrc
    TOP_MODULE ${CMAKE_CURRENT_SOURCE_DIR}/vsrc/Accum.v)
