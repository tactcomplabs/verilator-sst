#!/bin/bash
set -e
DEBUG=""
LINKHANDLE=""
CLOCKHANDLE=""
while getopts "hdlct:m:s:f:" flag
do
    case "${flag}" in
        h) echo "Usage: ./build.sh [OPTIONS] [-t <test_dir_name>] [-m <module_name> -s <source_dir> -f <source_files>]"
           echo -e "\nOptional flags:"
           echo "   -d                 : sets cmake build type to debug"
           echo "   -l                 : includes automatic link generation based on port names"
           echo -e "\nTest mode: build a testbench directory"
           echo "   -t <test_dir_name> : Accum, Accum1D, Array_IO, Counter, Scratchpad, UART, WIDE_IO"
           echo -e "\nCustom mode: build a custom verilog module. Ignored when -t is defined"
           echo "   -m <module_name>   : verilog top module"
           echo "   -s <source_dir>    : verilog source directory"
           echo "   -f <source_files>  : verilog source files"
           exit 1;;
        d) DEBUG="-DCMAKE_BUILD_TYPE=Debug";;
        l) LINKHANDLE="-DENABLE_LINK_HANDLING=ON";;
        c) CLOCKHANDLE="-DENABLE_CLK_HANDLING=ON";;
        t) TEST_MOD=${OPTARG};;
        m) MODULE=${OPTARG};;
        s) SOURCE_DIR=${OPTARG};;
        f) SOURCE_FILES=${OPTARG};;
    esac
done
if [ -d "$PWD/tests/$TEST_MOD" ]; then
    echo "Generating example module $TEST_MOD"
    VSOURCE="$PWD/tests/$TEST_MOD/vsrc"
    VDEVICE="$TEST_MOD"
    VSRCS="$PWD/tests/$TEST_MOD/vsrc/*"
    VTOP="$TEST_MOD"
elif [ -z "$TEST_MOD" ]; then
    echo "Generating custom module. . ."
    if [ -z "$SOURCE_DIR" ]; then
        echo "Error: Source directory must be defined with -s <source_dir> to build"
        exit 1
    elif [ -z "$MODULE" ]; then
        echo "Error: Module name must be defined with -m <module_name> to build"
        exit 1
    elif [ -z "$SOURCE_FILES" ]; then
        echo "Error: Source files must be defined with -f <source_files> to build"
        exit 1
    fi
    VSOURCE="$SOURCE_DIR"
    VDEVICE="$MODULE"
    VSRCS="$SOURCE_FILES"
    VTOP="$MODULE"
else
    echo "Error: built-in test module not recognized; options for use with -t flag are counter, accum, accum1D, uart, scratchpad"
    exit 1
fi
#cd $(dirname $0)
rm -rf build
mkdir build
cd build
cmake \
    $DEBUG \
    -DVERILOG_SOURCE=$VSOURCE \
    -DVERILOG_DEVICE=$VDEVICE \
    -DVERILOG_TOP_SOURCES=$VSRCS \
    -DVERILOG_TOP=$VTOP \
    $LINKHANDLE \
    $CLOCKHANDLE \
    ../
make VERBOSE=1 2>&1 | tee make.log
make VERBOSE=1 install 2>&1 | tee make.install.log
if [ -n "$TEST_MOD" ]; then
echo "To test the generated module using the built-in test component, run:"
echo "sst --model-options='-m $VTOP' tests/test_elements/verilator-test-direct/sample.py"
fi
cd ..
exit 0
