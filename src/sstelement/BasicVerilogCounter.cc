//
// _BasicVerilogCounter_cc_
//

#include <memory>
#include <sst/core/sst_config.h>
#include "BasicVerilogCounter.h"

#define LOW 0
#define HIGH 1

using namespace SST::VerilatorSST;

BasicVerilogCounter::BasicVerilogCounter(ComponentId_t id, Params& params)
  : Component(id) {
  out = new Output("", 1, 0, Output::STDOUT);
  const std::string clockFreq = params.find<std::string>("CLOCK_FREQ", "1GHz");
  uint16_t stop = params.find<std::uint16_t>("STOP", "20");

  verilatorSetup(stop);
  registerAsPrimaryComponent();
  primaryComponentDoNotEndSim();

  registerClock(clockFreq, new Clock::Handler<BasicVerilogCounter>(this, &BasicVerilogCounter::clock));
  out->output("Registering clock with frequency=%s\n", clockFreq.c_str());
  out->output("Counter set to stop=%u\n", stop);
}

void BasicVerilogCounter::verilatorSetup(uint16_t stop){
  top = std::make_unique<VerilatorSST>();

  Signal init_low(1,LOW);
  Signal init_stop(8,stop);

  top->writePort("clk", init_low);
  top->writePort("reset_l", init_low);
  top->writePort("stop", init_stop);
}

BasicVerilogCounter::~BasicVerilogCounter(){
  delete out;
}

bool BasicVerilogCounter::testBenchPass(){
  Signal done;
  top->readPort("done", done);

  bool pass = done.getUIntValue<uint8_t>() == HIGH;
  if(pass){
    top->finish();
    primaryComponentOKToEndSim();
  }

  return pass;
}

bool BasicVerilogCounter::clock(Cycle_t cycles){
  if(testBenchPass()){
    return true;
  }

  top->clockTick(1, "clk");
  if(top->getCurrentTick() > 5){
    Signal high(1,HIGH);
    top->writePort("reset_l", high);
  }

  return false;
}
