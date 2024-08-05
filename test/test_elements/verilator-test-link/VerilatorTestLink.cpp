//
// _VerilatorTestLink_cpp_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "VerilatorTestLink.h"
#include <fstream>

namespace SST::VerilatorSST{

VerilatorTestLink::VerilatorTestLink(SST::ComponentId_t id,
                                     const SST::Params& params )
  : SST::Component( id ), primaryComponent(false), NumCycles(1000),
    model(nullptr){

  const int Verbosity = params.find<int>( "verbose", 0 );
  output.init( "VerilatorTestLink[" + getName() + ":@p:@t]: ",
               Verbosity, 0, SST::Output::STDOUT );

  model = loadUserSubComponent<VerilatorSSTBase>("model");
  if( !model ){
    primaryComponent = true;
    output.verbose( CALL_INFO, 1, 0, "Registering as the primary component\n");
  }else{
    output.verbose( CALL_INFO, 1, 0, "Registering as the Verilator host component\n");
  }
  const int NumPorts = params.find<int>( "num_ports", 0 );
  InfoVec.resize( NumPorts );
  InitLinkConfig( params );
  InitPortMap( params );
  InitTestOps( params );

  const std::string clockFreq = params.find<std::string>( "clockFreq", "1GHz" );
  registerClock( clockFreq, new Clock::Handler<VerilatorTestLink>( this,
                                                                   &VerilatorTestLink::clock ) );

  NumCycles = params.find<uint64_t>( "numCycles", 1000 );

  if( primaryComponent) {
    registerAsPrimaryComponent();
    primaryComponentDoNotEndSim();
  }

  output.verbose( CALL_INFO, 1, 0, "Model construction complete\n" );
}

VerilatorTestLink::~VerilatorTestLink(){
  delete [] Links;
}

void VerilatorTestLink::setup(){
}

void VerilatorTestLink::finish(){
}

void VerilatorTestLink::init( unsigned int phase ){
  if( model ){
    output.verbose( CALL_INFO, 1, 0, "Initializing the Verilator model\n");
    model->init(phase);
  }
}

void VerilatorTestLink::InitPortMap( const SST::Params& params ) {
  std::vector<std::string> optList;
  params.find_array( "portMap", optList );
  for( size_t i=0; i<optList.size(); i++ ){
    output.verbose( CALL_INFO, 1, 0, "Port map entry: %s\n", optList[i].c_str() );
    std::vector<std::string> vstr;
    const std::string s = optList[i];
    splitStr(s, ':', vstr);
    if( vstr.size() != 4 ){
      output.fatal(CALL_INFO, -1,
                    "Error in reading value from portMap parameter:%s\n",
                    s.c_str() );
    }
    const long unsigned portId = std::stoul( vstr[1] );
    const long unsigned portSize = std::stoul( vstr[2] );
    const long unsigned portDirection = std::stoul( vstr[3] );
    PortMap[vstr[0]] = PortDef( portId, portSize, portDirection != 0 ); // 0 should mean reading, 1 should mean writing
    InfoVec[portId].PortId = portId;
    InfoVec[portId].Size = portSize; 
    InfoVec[portId].Write = ( portDirection != 0 );
  }
}

void VerilatorTestLink::InitLinkConfig( const SST::Params& params ) {
  const int NumPorts = params.find<int>( "num_ports", 0 );
  if ( NumPorts > 0 ) {
    Links = new SST::Link *[NumPorts];
    for (size_t i=0; i<NumPorts; i++) {
      char PortName[8];
      std::snprintf(PortName, 7, "port%zu", i);
      Links[i] = configureLink( PortName, "0ns", new Event::Handler<VerilatorTestLink, unsigned>( this, &VerilatorTestLink::RecvPortEvent, i ) );
      if ( Links[i] == nullptr ) {
        output.fatal( CALL_INFO, -1, "Error: Link for port %s failed to be configured\n", PortName );
      }
    }
  } else {
    output.fatal( CALL_INFO, -1, "Error: initialized with no links\n" );
  }
}

void VerilatorTestLink::InitTestOps( const SST::Params& params ) {
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

bool VerilatorTestLink::ExecTestOp() {
  if ( !OpQueue.empty() ) {
    const TestOp currOp = OpQueue.front();
    const uint32_t portId = currOp.PortId;
    const bool writing = InfoVec[portId].Write;
    uint32_t size = InfoVec[portId].Size;
    const uint32_t nvals = size / 8;
    const uint64_t tick = currOp.AtTick; 
    if ( currOp.AtTick > currTick ) {
      return false;
    } else if ( currOp.AtTick < currTick ) {
      output.fatal(CALL_INFO, -1,
                    "Error: TestOp detected past when it should've been executed. PortId=%" PRIu32 ", Tick=%" PRIu64 "\n",
                    portId, tick );
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
      output.verbose( CALL_INFO, 4, 0, "Sending write on port%" PRIu32 ": size=%" PRIu32 "\n", portId, InfoVec[portId].Size );
      for (size_t i=0; i<Data.size(); i++) {
        output.verbose( CALL_INFO, 4, 0, "byte %zu: %" PRIx8 "\n", i, Data[i] );
      }
      PortEvent * const opEvent = new PortEvent( Data );
      Links[portId]->send( opEvent );
    } else {
      output.verbose( CALL_INFO, 4, 0, "Data to be checked: size=%zu\n", Data.size() );
      for (size_t i=0; i<Data.size(); i++) {
        output.verbose( CALL_INFO, 4, 0, "byte %zu: %" PRIx8 "\n", i, Data[i] );
      }
      ReadDataCheck.emplace( Data );
      PortEvent * const opEvent = new PortEvent();
      Links[portId]->send( opEvent );
    }
    OpQueue.pop();
  return true;
  }
  return false;
}

void VerilatorTestLink::splitStr(const std::string& s,
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

void VerilatorTestLink::RecvPortEvent( SST::Event* ev, unsigned portId ) {
  // should only be receiving read data
  PortEvent * readEvent = reinterpret_cast<PortEvent *>( ev );
  const std::vector<uint8_t>& ValidData = ReadDataCheck.front();
  const std::vector<uint8_t>& ReadData = readEvent->getPacket();
  output.verbose( CALL_INFO, 4, 0, "Read data: size=%zu\n", ReadData.size() );
  for (size_t i=0; i<ReadData.size(); i++) {
    output.verbose( CALL_INFO, 4, 0, "byte %zu: %" PRIx8 "\n", i, ReadData[i] );
  }
  if ( ValidData.size() != ReadData.size() ) {
    output.fatal(CALL_INFO, -1,
                  "Error: Read data from port%" PRIu32 " has incorrect size (%zu, should be %zu) at tick %" PRIu64 "\n",
                  portId, ValidData.size(), ReadData.size(), currTick );
  }
  for (size_t i=0; i<ValidData.size(); i++) {
    if ( ValidData[i] != ReadData[i] ) {
      output.fatal(CALL_INFO, -1,
                    "Error: Read data from port%" PRIu32 " has incorrect value (%" PRIu8 ", should be %" PRIu8 ") at tick %" PRIu64 "\n",
                    portId, ReadData[i], ValidData[i], currTick );
    }
  }
  delete ev;
  ReadDataCheck.pop();
}

bool VerilatorTestLink::clock(SST::Cycle_t currentCycle){
  output.verbose( CALL_INFO, 4, 0, "Clocking cycle %" PRIu64 "\n", currentCycle );
  if( currentCycle > NumCycles ){
    output.verbose( CALL_INFO, 4, 0, "Cycle limit reached; ending sim\n" );
    primaryComponentOKToEndSim();
    return true;
  }
  // drive the test links (including the clock)
  while ( ExecTestOp() ); 
  currTick++;

  return false;
}

} // namespace SST::VerilatorSST

// EOF