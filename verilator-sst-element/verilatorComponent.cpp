//
// _VerilatorComponent_cpp_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "verilatorComponent.h"

namespace SST::VerilatorSST{
VerilatorComponent::VerilatorComponent(SST::ComponentId_t id,
                                         const SST::Params& params )
  : SST::Component( id ), NumCycles(1000), model(nullptr){

  const int Verbosity = params.find<int>( "verbose", 0 );
  output.init( "VerilatorComponent[" + getName() + ":@p:@t]: ",
               Verbosity, 0, SST::Output::STDOUT );

  model = loadUserSubComponent<VerilatorSSTBase>("model");
  if( !model ){
    output.fatal( CALL_INFO, -1, "Error: could not load model\n" );
  }

  const std::string clockFreq = params.find<std::string>( "clockFreq", "1GHz" );
  registerClock( clockFreq, new Clock::Handler<VerilatorComponent>( this,
                                                                     &VerilatorComponent::clock ) );

  NumCycles = params.find<uint64_t>("numCycles", 1000);

  registerAsPrimaryComponent();
  primaryComponentDoNotEndSim();

  output.verbose( CALL_INFO, 1, 0, "Model construction complete\n" );
}

VerilatorComponent::~VerilatorComponent(){
}

void VerilatorComponent::setup(){
}

void VerilatorComponent::finish(){
}

void VerilatorComponent::init( unsigned int phase ){
  model->init(phase);
}

bool VerilatorComponent::clock(SST::Cycle_t currentCycle){
  if( currentCycle > NumCycles ){
    primaryComponentOKToEndSim();
    return true;
  }

  return false;
}

} // namespace SST::VerilatorSST

// EOF
