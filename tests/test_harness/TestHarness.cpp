// tests/test_harness/TestHarness.cpp
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details

#include "TestHarness.h"

namespace SST::VerilatorSST{
TestHarness::TestHarness(SST::ComponentId_t id, const SST::Params& params)
    : SST::Component( id ),
    dut(nullptr){
  
    // log setup
    const int verbose = params.find<int>( "verbose", 0 );
    output = new SST::Output("TestHarness[@p:@l:@t]: ", verbose, 0, SST::Output::STDOUT);

    // device under test setup
    dut = loadUserSubComponent<VerilatorSSTBase>("dut");
    if( !dut ){
        output->fatal( CALL_INFO, -1, "Error: could not load dut\n" );
    }

    // clock setup
    const std::string clockFreq = params.find<std::string>( "clockFreq", "1GHz" );
    registerClock(clockFreq, new Clock::Handler<TestHarness>(this, &TestHarness::clock ) );
    numCycles = params.find<uint64_t>("numCycles", 1000);
    registerAsPrimaryComponent();
    primaryComponentDoNotEndSim();

    output->verbose( CALL_INFO, 1, 0, "Model construction complete\n" );
}

TestHarness::~TestHarness(){
    delete output;
    delete dut;
}

void TestHarness::setup(){
}

void TestHarness::finish(){
}

void TestHarness::init( unsigned int phase ){
  dut->init(phase);
}

std::vector<uint8_t> TestHarness::generateRandomData(unsigned width, unsigned depth){
  std::vector<uint8_t> data;

  // determine the number of uint8_t's
  unsigned numVals = (width/8) + ((width%8) > 0);

  for (unsigned j = 0; j < depth; j++) {
    // build the numVals-1 values
    unsigned currentBits = 0;
    for (unsigned i = 0; i < numVals - 1; i++) {
      data.push_back((uint8_t)(rand() & 0b11111111));
      currentBits += 8;
    }
    // build remaining bits
    uint8_t val = 0x0;
    for (unsigned i = 0; i < (width - currentBits); i++) {
      if ((rand() % 2) == 0) {
        val |= (1 << i);
      }
    }
    data.push_back(val);
  }

  return data;
}

void TestHarness::testDefault() {
  unsigned width;
  unsigned depth;
  SST::VerilatorSST::VPortType portType;
  std::vector<std::string> ports = dut->getPortsNames();
  const int verboseLevel = output->getVerboseLevel();

  for(const auto &port : ports){
    // get port info
    CHECK_TRUE(dut->getPortWidth(port, width));
    CHECK_TRUE(dut->getPortDepth(port, depth));
    CHECK_TRUE(dut->getPortType(port, portType));

    const uint32_t bits = width * depth;
    std::vector<uint8_t> data;

    //write to input port
    if (portType == SST::VerilatorSST::VPortType::V_INPUT) {
      data = generateRandomData(width, depth);
      dut->writePort(port, data);
      output->verbose(CALL_INFO,1,0,"Write: width=%u depth=%u port=%s data=",width,depth,port.c_str());
    }

    //read from output port
    if (portType == SST::VerilatorSST::VPortType::V_OUTPUT) {
      data = dut->readPort(port);
      output->verbose(CALL_INFO,1,0,"Read:  width=%u depth=%u port=%s data=",width,depth,port.c_str());
    }

    //output data
    if (verboseLevel >= 1) {
      for (const auto &d : data) {
        output->output("%x",static_cast<uint8_t>(d));
      }
      output->output("\n");
    }
  }
}

bool TestHarness::clock(SST::Cycle_t currentCycle){
    if( currentCycle > numCycles ){
        primaryComponentOKToEndSim();
        return true;
    }

    runTestSuite(currentCycle);
    return false;
}

}
