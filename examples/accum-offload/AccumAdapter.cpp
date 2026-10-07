//
// _AccumAdapter_cpp_
//
// Copyright (C) 2017-2026 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "AccumAdapter.h"

namespace SST::VerilatorSST {

AccumAdapter::AccumAdapter(SST::ComponentId_t id, const SST::Params &params)
    : AccumAccelAPI(id) {
  out.init("AccumAdapter[" + getName() + ":@p:@t]: ",
           params.find<int>("verbose", 0), 0, SST::Output::STDOUT);

  model = loadUserSubComponent<VerilatorSSTBase>("model");
  if (!model)
    out.fatal(CALL_INFO, -1, "no subcomponent loaded in slot \"model\"\n");

  engine = std::make_unique<AccumEngine>(out, model);

  // A subcomponent can own a clock just like a component. This one only
  // steps the RTL handshake; the model's own clock toggles `clk`.
  registerClock(params.find<std::string>("clockFreq", "1GHz"),
                new Clock::Handler<AccumAdapter, &AccumAdapter::clock>(this));
}

// Nobody calls init() on a subcomponent but its parent, so the chain is
// StubCpu::init -> AccumAdapter::init -> model->init.
void AccumAdapter::init(unsigned int phase) { engine->init(phase); }

void AccumAdapter::finish() {
  out.verbose(CALL_INFO, 1, 0, "completed %" PRIu64 " requests, %zu left\n",
              engine->completedCount(), engine->outstanding());
}

void AccumAdapter::submit(const Operands &ops, Callback done) {
  engine->submit(ops, std::move(done));
}

bool AccumAdapter::clock(SST::Cycle_t) {
  engine->step();
  return false;
}

} // namespace SST::VerilatorSST

// EOF
