#!/bin/bash

# ./gen_test.sh [test_category] [test_name]

set -e
TEMPLATE_TEST_DIR=$(realpath $(dirname $0))
VERILATORSST_TESTS_DIR=$(realpath $TEMPLATE_TEST_DIR/..)

# store arguments
TEST_CATEGORY="$1"
TEST_NAME="$2"
TEST_NAME_LOWERCASE="$(tr '[:upper:]' '[:lower:]' <<< ${TEST_NAME})"
TEST_NAME_PASCALCASE="$(tr '[:lower:]' '[:upper:]' <<< ${TEST_NAME:0:1})${TEST_NAME:1}"
export TEST_BENCH_CLASS="${TEST_NAME_PASCALCASE}TestBench"

# verify arguments
VALID_CATEGORIES="api_tests functional_tests integration_tests"
[[ $VALID_CATEGORIES =~ (^| )$TEST_CATEGORY($| ) ]] || ( printf  "$TEST_CATEGORY is not a valid test category:\n$VALID_CATEGORIES\n" && exit 1 )

# create test directory
mkdir -p $VERILATORSST_TESTS_DIR/$TEST_CATEGORY/$TEST_NAME_LOWERCASE/csrc \
         $VERILATORSST_TESTS_DIR/$TEST_CATEGORY/$TEST_NAME_LOWERCASE/vsrc
cp $TEMPLATE_TEST_DIR/vsrc/DUT.sv $VERILATORSST_TESTS_DIR/$TEST_CATEGORY/$TEST_NAME_LOWERCASE/vsrc

# configure test files
envsubst '$TEST_BENCH_CLASS' < $TEMPLATE_TEST_DIR/csrc/TestBench.cpp > $VERILATORSST_TESTS_DIR/$TEST_CATEGORY/$TEST_NAME_LOWERCASE/csrc/$TEST_BENCH_CLASS.cpp
envsubst '$TEST_BENCH_CLASS' < $TEMPLATE_TEST_DIR/csrc/TestBench.h > $VERILATORSST_TESTS_DIR/$TEST_CATEGORY/$TEST_NAME_LOWERCASE/csrc/$TEST_BENCH_CLASS.h
envsubst '$TEST_BENCH_CLASS' < $TEMPLATE_TEST_DIR/run_test.py > $VERILATORSST_TESTS_DIR/$TEST_CATEGORY/$TEST_NAME_LOWERCASE/run_test.py
envsubst '$TEST_BENCH_CLASS' < $TEMPLATE_TEST_DIR/CMakeLists.txt > $VERILATORSST_TESTS_DIR/$TEST_CATEGORY/$TEST_NAME_LOWERCASE/CMakeLists.txt

# add generated test to cmake build
echo "add_subdirectory($TEST_NAME_LOWERCASE)" >> $VERILATORSST_TESTS_DIR/$TEST_CATEGORY/CMakeLists.txt
