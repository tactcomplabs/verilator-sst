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
  std::cout << "init" << std::endl;
  out = new Output("", 1, 0, Output::STDOUT);
  clockFreq  = params.find<std::string>("CLOCK_FREQ", "1GHz");
  stop = params.find<std::uint32_t>("STOP", "20");

  verilatorSetup();
  registerAsPrimaryComponent();
  primaryComponentDoNotEndSim();

  registerClock(clockFreq, new Clock::Handler<BasicVerilogCounter>(this, &BasicVerilogCounter::clock));
  out->output("Registering clock with frequency=%s\n", clockFreq.c_str());
  out->output("Counter set to stop=%u\n", stop);
}

void BasicVerilogCounter::verilatorSetup(){
  std::cout << "verilatorSetup() start" << std::endl;
  std::function<void()> myfunc = [this]() {primaryComponentOKToEndSim();};
  top = std::make_unique<VerilatorSST>(myfunc);
  /////////DEBUG/////////
  std::cout << "DEBUG"<< std::endl;
  const uint8_t debug_val1 = 10;
  const uint16_t debug_val2 = 10;
  const float debug_val3 = -3.14;
  top->writePort<uint16_t>("stop", debug_val2);
  /////////DEBUG/////////
  const uint8_t init_low = 0;
  top->writePort("clk", init_low);
  top->writePort("reset_l", init_low);



  std::cout << "verilatorSetup() finish" << std::endl;
}

BasicVerilogCounter::~BasicVerilogCounter(){
  delete out;
}

bool BasicVerilogCounter::clock(Cycle_t cycles){
  top->tick(1, "clk");
  const uint8_t high = 1;
  if(top->getCurrentTick() > 5){
    top->writePort("reset_l", high);
  }

  uint8_t done;
  top->readPort("done",done);

  if(done == HIGH){
    top->finish();
    return true;
  }
  return false;
}
