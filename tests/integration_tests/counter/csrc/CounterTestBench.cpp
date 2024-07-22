// tests/integration_test/counter/csrc/CounterTestBench.cpp
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details

#include "CounterTestBench.h"

using namespace SST::VerilatorSST;

void CounterTestBench::runTestSuite(SST::Cycle_t cycle){
    testDefault();
}
