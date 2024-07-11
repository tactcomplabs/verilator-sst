//
// _VerilatorTestDirect_cpp_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "VerilatorTestDirect.h"

namespace SST::VerilatorSST{
VerilatorTestDirect::VerilatorTestDirect(SST::ComponentId_t id,
                                         const SST::Params& params )
  : SST::Component( id ), NumCycles(1000), model(nullptr){

  const int Verbosity = params.find<int>( "verbose", 0 );
  output.init( "VerilatorTestDirect[" + getName() + ":@p:@t]: ",
               Verbosity, 0, SST::Output::STDOUT );

  model = loadUserSubComponent<VerilatorSSTBase>("model");
  if( !model ){
    output.fatal( CALL_INFO, -1, "Error: could not load model\n" );
  }

  const std::string clockFreq = params.find<std::string>( "clockFreq", "1GHz" );
  registerClock( clockFreq, new Clock::Handler<VerilatorTestDirect>( this,
                                                                     &VerilatorTestDirect::clock ) );

  NumCycles = params.find<uint64_t>("numCycles", 1000);

  registerAsPrimaryComponent();
  primaryComponentDoNotEndSim();

  output.verbose( CALL_INFO, 1, 0, "Model construction complete\n" );
}

VerilatorTestDirect::~VerilatorTestDirect(){
}

void VerilatorTestDirect::setup(){
}

void VerilatorTestDirect::finish(){
}

void VerilatorTestDirect::init( unsigned int phase ){
  model->init(phase);
}

std::vector<uint8_t> VerilatorTestDirect::generateData(unsigned Width, unsigned Depth){
  std::vector<uint8_t> data;

  // determine the number of uint8_t's
  unsigned NumVals = (Width/8) + ((Width%8) > 0);

  for (unsigned j = 0; j < Depth; j++) {
    // build the NumVals-1 values
    unsigned CurrentBits = 0;
    for (unsigned i = 0; i < NumVals - 1; i++) {
      data.push_back((uint8_t)(rand() & 0b11111111));
      CurrentBits += 8;
    }
    // build remaining bits
    uint8_t val = 0x0;
    for (unsigned i = 0; i < (Width - CurrentBits); i++) {
      if ((rand() % 2) == 0) {
        val |= (1 << i);
      }
    }
    data.push_back(val);
  }

  return data;
}

void VerilatorTestDirect::testSuite() { 
  testPortNames();
  testNumPorts();
  testPortWidth();
  testPortDepth();
  testPortType();
  testPortWrite();
  testPortRead();
  testPortWriteRead();
}

bool VerilatorTestDirect::clock(SST::Cycle_t currentCycle){
  if( currentCycle > NumCycles ){
    primaryComponentOKToEndSim();
    return true;
  }

  testSuite();

  // build a new set of wire payloads
  std::vector<std::string> Ports = model->getPortsNames();
  unsigned Width;
  unsigned Depth;
  SST::VerilatorSST::VPortType Type;
  for( auto i : Ports ){
    if( !model->getPortWidth(i, Width) ){
      output.fatal(CALL_INFO, -1, "Could not retrieve width of port=%s\n",
                   i.c_str());
    }
    if( !model->getPortDepth(i, Depth) ){
      output.fatal(CALL_INFO, -1, "Could not retrieve depth of port=%s\n",
                   i.c_str());
    }
    if( !model->getPortType(i, Type) ){
      output.fatal(CALL_INFO, -1, "Could not retrieve type of port=%s\n",
                   i.c_str());
    }
    uint32_t bits = Width * Depth;
    std::vector<uint8_t> data;
    if (Type == SST::VerilatorSST::VPortType::V_INPUT) {
      // write to the port
      data = generateData(Width, Depth);
      if (bits < 9) {
        output.verbose(CALL_INFO, 2, 0, "Data generated (W%dD%d) for sending on port %s: %02x\n", Width, Depth, i.data(), data[0]);
      }
      else if (bits < 33) {
        output.verbose(CALL_INFO, 2, 0, "Data generated (W%dD%d) for sending on port %s: %02x %02x %02x %02x\n", Width, Depth, i.data(), data[0], data[1], data[2], data[3]);
      }
      else {
        output.verbose(CALL_INFO, 2, 0, "Data generated (W%dD%d) for sending on port %s: %02x %02x %02x %02x %02x %02x %02x %02x\n", Width, Depth, i.data(), data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
      }
      model->writePort(i, data);
    }
    else {
      // read from the port
      data = model->readPort(i);
      if (bits < 9) {
        output.verbose(CALL_INFO, 2, 0, "Data received (W%dD%d) from port %s: %02x\n", Width, Depth, i.data(), data[0]);
      }
      else if (bits < 33) {
        output.verbose(CALL_INFO, 2, 0, "Data received (W%dD%d) from port %s: %02x %02x %02x %02x\n", Width, Depth, i.data(), data[0], data[1], data[2], data[3]);
      }
      else {
        output.verbose(CALL_INFO, 2, 0, "Data received (W%dD%d) from port %s: %02x %02x %02x %02x %02x %02x %02x %02x\n", Width, Depth, i.data(), data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
      }
    }
  }

  return false;
}

} // namespace SST::VerilatorSST

// EOF
