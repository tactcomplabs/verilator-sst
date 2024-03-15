//
// _BasicVerilogCounter_cc_
//

#include <memory>
// #include <verilated.h>
// #include "Top.h"

#include <sst/core/sst_config.h>
#include "BasicVerilogCounter.h"

using namespace SST::VerilatorSST;

BasicVerilogCounter::BasicVerilogCounter(ComponentId_t id, Params& params)
  : Component(id) {

  out = new Output("", 1, 0, Output::STDOUT);
  clockFreq  = params.find<std::string>("clockFreq", "1GHz");
  stop = params.find<std::uint64_t>("stop", "20");

  verilatorSetup();
  registerAsPrimaryComponent();
  primaryComponentDoNotEndSim();

  registerClock(clockFreq, new Clock::Handler<BasicVerilogCounter>(this, &BasicVerilogCounter::clock));
  out->output("Registering clock with frequency=%s\n", clockFreq.c_str());
  out->output("Counter set to stop=%u\n", stop);
}

void BasicVerilogCounter::verilatorSetup(){
  top = std::make_unique<VerilatorSST>();
  uint8_t init_clk = 0;
  uint8_t init_reset_l = 0;
  PortType init_clk_port = PortType{&init_clk};
  PortType init_reset_l_port = PortType{&init_reset_l};

  top->writeInputPort("clk", init_clk_port);
  top->writeInputPort("reset_l", init_reset_l_port);
  top->writeInputPort("stop", PortType{&stop});

  // contextp = new VerilatedContext;
  // contextp->debug(0);
  // contextp->randReset(2);
  // contextp->traceEverOn(true);
  // char ** empty = {};
  // contextp->commandArgs(0, empty);

  // top = new Top;
  // top->reset_l = 0;
  // top->clk = 0;
  // top->stop = stop;
}

BasicVerilogCounter::~BasicVerilogCounter(){
  delete out;
}

bool BasicVerilogCounter::clock(Cycle_t cycles){
  // contextp->timeInc(1);
  // top->clk = !top->clk;
  // //clk period = 1/2 cycle period

  // if(!top->clk){
  //   if(contextp->time() > 5){
  //     top->reset_l = 1;
  //   }
  // }

  // top->eval();

  // if(top->done == 1){
  //   out->output("sst: done signal high\n");
  //   top->final();
  //   primaryComponentOKToEndSim();
  //   return true;
  // }else {
  //   return false;
  // }

  primaryComponentOKToEndSim();
  return true;
}
