//
// _basicComponent_cc_
//

#include <memory>
#include <verilated.h>
#include "Top.h"

#include <sst/core/sst_config.h>
#include "basicComponent.h"

using namespace SST;
using namespace SST::basicComponent;

basicVerilogCounter::basicVerilogCounter(ComponentId_t id, Params& params)
  : Component(id) {

  out = new Output("", 1, 0, Output::STDOUT);
  clockFreq  = params.find<std::string>("clockFreq", "1GHz");
  stop = params.find<std::uint64_t>("stop", "20");

  verilatorSetup();
  registerAsPrimaryComponent();
  primaryComponentDoNotEndSim();

  registerClock(clockFreq, new Clock::Handler<basicVerilogCounter>(this, &basicVerilogCounter::clock));
  out->output("Registering clock with frequency=%s\n", clockFreq.c_str());
  out->output("Counter set to stop=%u\n", stop);
}

void basicVerilogCounter::verilatorSetup(){
  contextp = new VerilatedContext;
  contextp->debug(0);
  contextp->randReset(2);
  contextp->traceEverOn(true);
  char ** empty = {};
  contextp->commandArgs(0, empty);

  top = new Top;
  top->reset_l = 0;
  top->clk = 0;
  top->stop = stop;
}

basicVerilogCounter::~basicVerilogCounter(){
  delete out;
}

bool basicVerilogCounter::clock(Cycle_t cycles){
  contextp->timeInc(1);
  top->clk = !top->clk;
  //clk period = 1/2 cycle period

  if(!top->clk){
    if(contextp->time() > 5){
      top->reset_l = 1;
    }
  }

  top->eval();

  if(top->done == 1){
    out->output("sst: done signal high\n");
    top->final();
    primaryComponentOKToEndSim();
    return true;
  }else {
    return false;
  }
}
