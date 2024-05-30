#!/bin/bash
set -ex
cd $(dirname $0)
rm ~/.sst/sstsimulator.conf
touch ~/.sst/sstsimulator.conf
rm -rf build install
mkdir build
INSTALL_PREFIX=$(realpath ./install)
cd build
cmake ..
make VERBOSE=1 install 2>&1 | tee make.install.log
