//
// _BigVerilogAccum_cc_
//

#include "BigVerilogAccum.h"

#define LOW (uint64_t) 0
#define HIGH (uint64_t) 1

using namespace SST::VerilatorSST;

BigVerilogAccum::BigVerilogAccum(ComponentId_t id, Params& params)
  : Component(id) {
  out = new Output("", 1, 0, Output::STDOUT);
  const std::string clockFreq = params.find<std::string>("CLOCK_FREQ", "1GHz");
  uint64_t add = params.find<std::uint64_t>("ADD", "64");
  uint64_t add2 = params.find<std::uint64_t>("ADD2", "64");
  accumWidth = params.find<std::uint16_t>("ACCUM_WIDTH", "64"); // after initial functionality, make >=128 to test bug
  dawsonCounter = 0;
  uint64_t comb_add[2];
  comb_add[0] = add2;
  comb_add[1] = add; // counter intuitive - lower bits are element 1 not element 0
  uint64_t * sel_add;

  if (accumWidth > 64) { //requires both add params
    sel_add = comb_add;
  } else { // requires only first add param
    sel_add = &(comb_add[1]);
  }

  verilatorSetup(sel_add);
  registerAsPrimaryComponent();
  primaryComponentDoNotEndSim();

  registerClock(clockFreq, new Clock::Handler<BigVerilogAccum>(this, &BigVerilogAccum::clock));
  out->output("Registering clock with frequency=%s\n", clockFreq.c_str());
  out->output("Accumulator width set to accumWidth=%u\n", accumWidth);
  out->output("Add set to add=%llx,%llx (long param)\n", comb_add[1], comb_add[0]);
}

BigVerilogAccum::~BigVerilogAccum(){
  delete out;
}

void BigVerilogAccum::verilatorSetup(uint64_t * add){
  top = std::make_unique<VerilatorSST<VTop>>();

  Signal init_low(1,LOW);
  //Signal init_add(accumWidth, *add); 
  Signal init_en(1,HIGH);
  Signal init_add((signal_width_t) accumWidth, add); // wide constructor -- takes array but no depth

  top->writePort("clk", init_low);
  top->writePort("reset_l", init_low);

  Signal reset_delay(1,HIGH);
  top->writePortAtTick("reset_l", reset_delay, 5);
  top->writePortAtTick("add", init_add, 8);
  top->writePortAtTick("en", init_en, 8);
  top->writePortAtTick("en", init_low, 12);
  top->tick(1);
}

bool BigVerilogAccum::testBenchPass(){
  Signal accumulator = top->readPort("accum");

  //uint64_t pass = accumulator.getUIntScalar<uint64_t>();
  uint64_t * pass = accumulator.getUIntWide<uint64_t>(); // wide accessor
  //if(pass >= 15U){
  if(pass[1] >= 15U){
    // beware the order; lower bits in RTL come out in the later element... maybe a TODO
    out->output("sst: test passed with val %llx,%llx!\n", pass[0], pass[1]);
    top->finish();
    //top->finish(); // tested remove, could test moving it
    return(true);
  }
  return (false);
}

bool BigVerilogAccum::clock(Cycle_t cycles){
  if(top->getCurrentTick() > 12){
    if(testBenchPass()){
      if (dawsonCounter != 100) {
        dawsonCounter++;
        return false;
      } else {
        primaryComponentOKToEndSim();
        return true;
      }
    }
  }
  top->tickClockPeriod("clk");
  return false;
}
