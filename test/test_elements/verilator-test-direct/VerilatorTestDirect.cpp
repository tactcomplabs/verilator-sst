//
// _VerilatorTestDirect_cpp_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "verilatorSSTAPI.h"
#include "VerilatorTestDirect.h"
#include <fstream>

namespace SST::VerilatorSST{
VerilatorTestDirect::VerilatorTestDirect(SST::ComponentId_t id,
                                         const SST::Params& params )
  : SST::Component( id ), NumCycles(1000), model(nullptr){

  const int Verbosity = params.find<int>( "verbose", 0 );
  const VerboseMasking VerbosityMask = static_cast<VerboseMasking>( params.find<uint32_t>( "verboseMask", 0 ) );
  output.init( "VerilatorTestDirect[" + getName() + ":@p:@t]: ",
               Verbosity, VerbosityMask, SST::Output::STDOUT );

  model = loadUserSubComponent<VerilatorSSTBase>("model");
  if( !model ){
    output.fatal( CALL_INFO, -1, "Error: could not load model\n" );
  }

  const std::string clockFreq = params.find<std::string>( "clockFreq", "1GHz" );
  registerClock( clockFreq, new Clock::Handler<VerilatorTestDirect>( this,
                                                                     &VerilatorTestDirect::clock ) );
  InitTestOps( params );

  NumCycles = params.find<uint64_t>("numCycles", 1000);

  registerAsPrimaryComponent();
  primaryComponentDoNotEndSim();

  output.verbose( CALL_INFO, 1, VerboseMasking::INIT, "Model construction complete\n" );
}

VerilatorTestDirect::~VerilatorTestDirect(){
}

void VerilatorTestDirect::setup(){
}

void VerilatorTestDirect::finish(){
}

void VerilatorTestDirect::init( unsigned int phase ){
  if ( model ) {
    output.verbose( CALL_INFO, 1, VerboseMasking::INIT, "Initializing the Verilator model\n");
    model->init(phase);
  }
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

void VerilatorTestDirect::InitTestOps( const SST::Params& params ) {
  const std::string fileName = params.find<std::string>( "testFile", "" );
  // test operations should have the form portname:value:tick
  if ( fileName == "" ) {
    // try load test ops from param here
    std::vector<std::string> optList;
    params.find_array<std::string>( "testOps", optList );
    for (auto it=optList.begin(); it!=optList.end(); it++) {
      const TestOp & toQueue = ConvertToTestOp( *it );
      OpQueue.push( toQueue );
    } 
  } else {
    std::ifstream testFile( fileName );
    std::string line;
    if ( testFile.is_open() ) {
      while ( std::getline( testFile, line) ) {
        const TestOp & toQueue = ConvertToTestOp( line );
        OpQueue.push( toQueue );
      }
    }
  }
}

bool VerilatorTestDirect::ExecTestOp() {
  if ( !OpQueue.empty() ) {
    const TestOp currOp = OpQueue.front();
    //const uint32_t portId = currOp.PortId;
    const std::string portName = currOp.PortName;
    //uint32_t size = InfoVec[portId].Size;
    uint32_t width;
    uint32_t depth;
    model->getPortWidth( portName, width );
    model->getPortDepth( portName, depth );
    const uint32_t byteWidth = width / 8 + ( ( width % 8 == 0 ) ? 0 : 1 );
    uint32_t size = byteWidth * depth;
    //const bool writing = InfoVec[portId].Write;
    VPortType portType;
    model->getPortType(portName, portType);
    const bool writing = currOp.isWrite;
    const uint32_t nvals = size / 8;
    const uint64_t tick = currOp.AtTick; 
    if ( currOp.AtTick > currTick ) {
      return false;
    } else if ( currOp.AtTick < currTick ) {
      output.fatal(CALL_INFO, -1,
                    "Error: TestOp detected past when it should've been executed. PortName=%s, Tick=%" PRIu64 "\n",
                   portName.c_str(), tick );
    }
    const uint64_t * vals = currOp.Values;
    std::vector<uint8_t> Data;
    for ( size_t i=0; i<nvals; i++ ) {
      AddToPacket<uint64_t>( vals[i], Data );
      size -= 8;
    }
    const uint8_t * currPtr = reinterpret_cast<const uint8_t *>( vals ) + ( 8 * nvals );
    if ( size > 4 ) {
      AddToPacket<uint32_t>( *reinterpret_cast<const uint32_t *>( currPtr ), Data );
      currPtr += 4;
      size -= 4;
    }
    if ( size > 2) {
      AddToPacket<uint16_t>( *reinterpret_cast<const uint16_t *>( currPtr ), Data );
      currPtr += 2;
      size -= 2;
    }
    if ( size > 0 ) {
      Data.push_back( *currPtr );
    }
    if ( writing ) {
      output.verbose( CALL_INFO, 4, VerboseMasking::WRITE_EVENT, "Sending write on port %s: size=%zu\n", portName.c_str(), Data.size() );
      for (size_t i=0; i<Data.size(); i++) {
        output.verbose( CALL_INFO, 4, VerboseMasking::WRITE_DATA, "byte %zu: %" PRIx8 "\n", i, Data[i] );
      }
      model->writePort(portName, Data);
    } else {
      output.verbose( CALL_INFO, 4, VerboseMasking::READ_EVENT, "Data to be checked: size=%zu\n", Data.size() );
      for (size_t i=0; i<Data.size(); i++) {
        output.verbose( CALL_INFO, 4, VerboseMasking::READ_DATA, "byte %zu: %" PRIx8 "\n", i, Data[i] );
      }
      const std::vector<uint8_t> & ReadData = model->readPort(portName);
      output.verbose( CALL_INFO, 4, VerboseMasking::READ_DATA, "Read data: size=%zu\n", ReadData.size() );
      for (size_t i=0; i<ReadData.size(); i++) {
        output.verbose( CALL_INFO, 4, VerboseMasking::READ_DATA, "byte %zu: %" PRIx8 "\n", i, ReadData[i] );
      }
      if ( Data.size() != ReadData.size() ) {
        output.fatal(CALL_INFO, -1,
                     "Error: Read data from port %s has incorrect size (%zu, should be %zu) at tick %" PRIu64 "\n",
                     portName.c_str(), Data.size(), ReadData.size(), currTick );
      }
      for (size_t i=0; i<Data.size(); i++) {
        if ( Data[i] != ReadData[i] ) {
          output.fatal(CALL_INFO, -1,
                    "Error: Read data from port %s has incorrect value (%" PRIu8 ", should be %" PRIu8 ") at tick %" PRIu64 "\n",
                    portName.c_str(), ReadData[i], Data[i], currTick );
        }
      }
    }
    OpQueue.pop();
  return true;
  }
  return false;
}

void VerilatorTestDirect::splitStr(const std::string& s,
                                            char c,
                                            std::vector<std::string>& v){
  std::string::size_type i = 0;
  std::string::size_type j = s.find(c);
  v.clear();

  if( (j==std::string::npos) && (s.length() > 0) ){
    v.push_back(s);
    return ;
  }

  while (j != std::string::npos) {
    v.push_back(s.substr(i, j-i));
    i = ++j;
    j = s.find(c, j);
    if (j == std::string::npos)
      v.push_back(s.substr(i, s.length()));
  }
}

bool VerilatorTestDirect::clock(SST::Cycle_t currentCycle){
  output.verbose( CALL_INFO, 4, VerboseMasking::CLOCK_INFO, "Clocking cycle %" PRIu64 "\n", currentCycle );
  if( currentCycle > NumCycles ){
    primaryComponentOKToEndSim();
    return true;
  }

  while ( ExecTestOp() );
  currTick++;

  return false;
}

} // namespace SST::VerilatorSST

// EOF
