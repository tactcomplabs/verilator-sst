#include "testHarness.h"

namespace SST::VerilatorSST{
TestHarness::TestHarness(SST::ComponentId_t id, const SST::Params& params)
    : SST::Component( id ),
    dut(nullptr){
  
    // log setup
    const int verbosity = params.find<int>( "verbosity", 0 );
    output = new SST::Output("TestHarness[@p:@l:@t]: ", verbosity, 0, SST::Output::STDOUT);

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

bool TestHarness::clock(SST::Cycle_t currentCycle){
    if( currentCycle > numCycles ){
        primaryComponentOKToEndSim();
        return true;
    }

    runTestSuite(currentCycle);
    return false;
}

}
