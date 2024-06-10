cmake_minimum_required(VERSION 3.13)

project(basicveriloguartmem)
find_package(verilatorsst HINTS ${CMAKE_CURRENT_SOURCE_DIR}/../..)
add_library(basicveriloguartmem SHARED "")
verilatorsstf(basicveriloguartmem
    VSRC_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/vsrc
    CSRC_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/csrc
    TOP_MODULE ${CMAKE_CURRENT_SOURCE_DIR}/vsrc/UART.sv
    VERILATOR_ARGS "-DADDR_WIDTH=8" "-DDATA_WIDTH=16" "-DFRAME_WIDTH=16" "-DBAUD_PERIOD=16")
