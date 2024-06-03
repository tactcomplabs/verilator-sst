#!/bin/bash
set -ex
cd $(dirname $0)
rm -rf build
mkdir build
cd build
cmake ..
make VERBOSE=1 install 2>&1 | tee make.install.log
