#!/bin/bash
set -e
DEBUG=""
LINKHANDLE=""
CLOCKHANDLE=""
CLOCKNAME=""
while getopts "hdlcn:t:m:s:f:" flag
do
    case "${flag}" in
        h) echo "./build.sh -t {counter, accum, accum1D, uart} will build the selected example module"
           echo "For custom modules, -m <module_name> -s <source_dir> -f <source_files> must be defined"
           echo "-d : Sets cmake build type to Debug"
           echo "-l : Includes automatic link generation based on port names (cannot be used with automatic clock handling and should include clock port name)"
           echo "-n : Sets the top module's clock port name for discovery (should be used with automatic link generation)"
           echo "-c : Includes automatic clock handling (should only be used with subcomponent calls)"
           exit 1;;
        d) DEBUG="-DCMAKE_BUILD_TYPE=Debug";;
        l) LINKHANDLE="-DENABLE_LINK_HANDLING=ON"
            if [ "$CLOCKHANDLE" = "-DENABLE_CLK_HANDLING=ON" ]; then
            echo "Error: Automatic clock handling and automatic link generation cannot be used simultaneously"
            exit 1;
            fi
            ;;
        c) CLOCKHANDLE="-DENABLE_CLK_HANDLING=ON"
            if [ "$LINKHANDLE" = "-DENABLE_LINK_HANDLING=ON" ]; then
            echo "Error: Automatic clock handling and automatic link generation cannot be used simultaneously"
            exit 1;
            fi
            ;;
        n) CLOCKNAME="-DCLOCK_PORT_NAME=${OPTARG}";;
        t) TEST_MOD=${OPTARG};;
        m) MODULE=${OPTARG};;
        s) SOURCE_DIR=${OPTARG};;
        f) SOURCE_FILES=${OPTARG};;
    esac
done
if [ "$TEST_MOD" = "counter" ]; then
    echo "Generating example module counter"
    VSOURCE="$PWD/tests/counter/"
    VDEVICE="Counter"
    VSRCS="$PWD/tests/counter/Counter.v"
    VTOP="Counter"
elif [ "$TEST_MOD" = "accum" ]; then
    echo "Generating example module accum"
    VSOURCE="$PWD/tests/accum/"
    VDEVICE="Accum"
    VSRCS="$PWD/tests/accum/Accum.sv"
    VTOP="Accum"
elif [ "$TEST_MOD" = "accum1D" ]; then
    echo "Generating example module accum1D"
    VSOURCE="$PWD/tests/accum/"
    VDEVICE="Accum"
    VSRCS="$PWD/tests/accum/Accum1D.sv"
    VTOP="Accum"
elif [ "$TEST_MOD" = "uart" ]; then
    echo "Generating example module uart"
    VSOURCE="$PWD/tests/uart_mem/"
    VDEVICE="UART"
    VSRCS="$PWD/tests/uart_mem/*"
    VTOP="UART"
elif [ "$TEST_MOD" = "scratchpad" ]; then
    echo "Generating example module scratchpad"
    VSOURCE="$PWD/tests/scratchpad/"
    VDEVICE="Scratchpad"
    VSRCS="$PWD/tests/scratchpad/*"
    VTOP="Scratchpad"
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
    $CLOCKNAME \
    ../
make VERBOSE=1 | tee make.log
make VERBOSE=1 install | tee make.install.log
if [ -n "$TEST_MOD" ]; then
    echo "To test the generated module using the built-in test component, run:"
    if [ "$LINKHANDLE" = "-DENABLE_LINK_HANDLING=ON" ]; then
        echo "sst --model-options='-m $VTOP' tests/test_elements/verilator-test-link/sample.py"
    else
        echo "sst --model-options='-m $VTOP' tests/test_elements/verilator-test-direct/sample.py"
    fi
fi
cd ..
exit 0
