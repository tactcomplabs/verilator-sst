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

  const std::string clockFreq = params.find<std::string>( "clockFreq", "1GHz" );
  registerClock( clockFreq, new Clock::Handler<VerilatorTestLink>( this,
                                                                   &VerilatorTestLink::clock ) );

  NumCycles = params.find<uint64_t>("numCycles", 1000);

  if( primaryComponent) {
    registerAsPrimaryComponent();
    primaryComponentDoNotEndSim();
  }

  output.verbose( CALL_INFO, 1, 0, "Model construction complete\n" );
}

VerilatorTestLink::~VerilatorTestLink(){
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

bool VerilatorTestLink::clock(SST::Cycle_t currentCycle){
  if( currentCycle > NumCycles ){
    primaryComponentOKToEndSim();
    return true;
  }

  // drive the test links (including the clock)

  return false;
}

} // namespace SST::VerilatorSST

// EOF
