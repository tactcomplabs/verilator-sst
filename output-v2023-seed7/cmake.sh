#!/bin/bash

cmake \
    -DENABLE_CUSTOM_MODULE=ON \
    -DVERILOG_SOURCE_DIR=./bambu-out \
    -DVERILOG_DEVICE=forwardKernelTB \
    -DVERILOG_TOP=forward_kernel_tb \
    -DVERILOG_TOP_SOURCES=testbench_forward_kernel_tb.v \
    -DVERILATOR_OPTIONS="-Wno-fatal -Wno-lint" \
    -DENABLE_LINK_HANDLING=ON \
    -DCLOCK_PORT_NAME=clock \
    ..
