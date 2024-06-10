//
// _BasicVerilogCounter_cc_
//

#include "BasicVerilogCounter.h"

using namespace SST::VerilatorSST;

BasicVerilogCounter::BasicVerilogCounter(ComponentId_t id, Params& params)
  : Component(id) {
  out = new Output("", 1, 0, Output::STDOUT);
  const std::string clockFreq = params.find<std::string>("CLOCK_FREQ", "1GHz");
  uint16_t stop = params.find<std::uint16_t>("STOP", "20");
  stopWidth = params.find<std::uint16_t>("STOP_WIDTH", "8");

  verilatorSetup(stop);
  registerAsPrimaryComponent();
  primaryComponentDoNotEndSim();

  registerClock(clockFreq, new Clock::Handler<BasicVerilogCounter>(this, &BasicVerilogCounter::clock));
  out->output("Registering clock with frequency=%s\n", clockFreq.c_str());
  out->output("Stop width set to stopWidth=%u\n", stopWidth);
  out->output("Counter set to stop=%u\n", stop);
}

BasicVerilogCounter::~BasicVerilogCounter(){
  delete out;
}

void BasicVerilogCounter::verilatorSetup(uint16_t stop){
  top = std::make_unique<VerilatorSST<VTop>>();

  Signal init_low(1,SIGNAL_LOW);
  Signal init_stop(stopWidth,stop);

  top->writePort("clk", init_low);
  top->writePort("reset_l", init_low);
  top->writePort("stop", init_stop);
  top->tick(1);

  Signal reset_delay(1,SIGNAL_HIGH);
  top->writePortAtTick("reset_l",reset_delay,5);
}

bool BasicVerilogCounter::testBenchPass(){
  Signal done = top->readPort("done");

  auto pass = done.getUIntScalar();
  if(pass & 1){
    out->output("sst: test passed!\n");
    top->finish();
    
  }

  return pass;
}

bool BasicVerilogCounter::clock(Cycle_t cycles){
  if(top->getCurrentTick() > 5){
    if(testBenchPass()){
      primaryComponentOKToEndSim();
      return true;
    }
  }
  top->tickClockPeriod("clk");
  return false;
}
