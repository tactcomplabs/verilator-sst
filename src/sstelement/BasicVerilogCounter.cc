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
  clockFreq  = params.find<std::string>("clockFreq", "1GHz");
  stop = params.find<std::uint32_t>("stop", "20");

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
  const uint8_t init_low = 0;
  top->writePort("clk", init_low);
  top->writePort("reset_l", init_low);
  top->writePort("stop", stop);
  std::cout << "verilatorSetup() finish" << std::endl;
}

BasicVerilogCounter::~BasicVerilogCounter(){
  delete out;
}

bool BasicVerilogCounter::clock(Cycle_t cycles){
  top->tick(1, "clk");
  if(top->getCurrentTick() > 5){
    top->writePort("reset_l", HIGH);
  }

  PortType done;
  top->readPort("done",done);

  bool ret = false;
  std::visit([&, this](auto&& arg) {
    if(*arg == HIGH){
      top->finish();
      ret = true;
    }
  },done);
  return ret;
}
