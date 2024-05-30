#!/bin/bash
set -ex
cd $(dirname $0)
rm -rf build
mkdir build
cd build
cmake .. \
    -DVERILOG_SOURCE=$PWD/../examples/BasicVerilogCounter/vsrc \
    -DVERILOG_DEVICE=counter \
    -DVERILOG_TOP=Counter \
    -DVERILOG_TOP_SOURCES=$PWD/../examples/BasicVerilogCounter/vsrc/Counter.v
make VERBOSE=1 2>&1 | tee make.log
make VERBOSE=1 install 2>&1 | tee make.install.log
