// tests/test_template/csrc/TestBench.cpp
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details

#include "$TEST_BENCH_CLASS.h"

using namespace SST::VerilatorSST;

void $TEST_BENCH_CLASS::runTestSuite(SST::Cycle_t cycle){
    testDefault();
}
