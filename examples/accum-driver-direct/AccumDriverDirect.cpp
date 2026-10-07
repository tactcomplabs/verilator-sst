//
// _AccumDriverDirect_cpp_
//
// Copyright (C) 2017-2026 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "AccumDriverDirect.h"

namespace SST::VerilatorSST {

AccumDriverDirect::AccumDriverDirect(SST::ComponentId_t id,
                                     const SST::Params &params)
    : SST::Component(id) {
  out.init("AccumDriverDirect[" + getName() + ":@p:@t]: ",
           params.find<int>("verbose", 0), 0, SST::Output::STDOUT);
  numOps = params.find<uint64_t>("numOps", 8);
  rng.seed(params.find<uint32_t>("seed", 1));

  model = loadUserSubComponent<VerilatorSSTBase>("model");
  if (!model)
    out.fatal(CALL_INFO, -1, "no subcomponent loaded in slot \"model\"\n");

  registerClock(params.find<std::string>("clockFreq", "1GHz"),
                new Clock::Handler<AccumDriverDirect,
                                   &AccumDriverDirect::clock>(this));

  registerAsPrimaryComponent();
  primaryComponentDoNotEndSim();
}

// SST does not call init() on subcomponents for us; forward it.
void AccumDriverDirect::init(unsigned int phase) { model->init(phase); }
void AccumDriverDirect::setup() {}

void AccumDriverDirect::finish() {
  const bool ok = errors == 0 && opsDone == numOps;
  out.output("AccumDriverDirect: %s (%" PRIu64 "/%" PRIu64 " checks, %" PRIu64
             " errors)\n",
             ok ? "PASSED" : "FAILED", opsDone, numOps, errors);
}

void AccumDriverDirect::checkAccum() {
  const auto bytes = model->readPort("accum");
  if (bytes.size() != 4 * Lanes)
    out.fatal(CALL_INFO, -1, "accum read returned %zu bytes, expected %u\n",
              bytes.size(), 4 * Lanes);
  for (unsigned i = 0; i < Lanes; i++) {
    uint32_t got = 0;
    for (unsigned b = 0; b < 4; b++)
      got |= uint32_t(bytes[i * 4 + b]) << (8 * b);
    out.verbose(CALL_INFO, 2, 0, "accum[%u] = 0x%08x (want 0x%08x)\n", i, got,
                expected[i]);
    if (got != expected[i]) {
      errors++;
      out.output("accum[%u] mismatch: got 0x%08x want 0x%08x\n", i, got,
                 expected[i]);
    }
  }
}

bool AccumDriverDirect::clock(SST::Cycle_t) {
  // No clk writes here: the model toggles its own clock. We only change
  // inputs and sample outputs, and advance on the `done` handshake:
  //   Reset:     pulse reset_l low (async reset in the RTL)
  //   Issue:     drive add[], en=1
  //   WaitDone:  RTL raised done -> accum[] is updated; check it, en=0
  //   WaitClear: RTL dropped done -> ready for the next op
  switch (state) {
  case State::Reset:
    // reset_l goes 1 -> 0 -> 1 over three driver cycles so the RTL sees a
    // falling edge regardless of its random power-up value
    if (cycle == 0) {
      writeBit("en", false);
      writeBit("reset_l", true);
    } else if (cycle == 1) {
      writeBit("reset_l", false);
    } else {
      writeBit("reset_l", true);
      state = State::Issue;
    }
    break;

  case State::Issue: {
    std::vector<uint8_t> packet;
    for (unsigned i = 0; i < Lanes; i++) {
      const uint16_t v = rng() & 0xffff;
      expected[i] += v;
      packet.push_back(v & 0xff);
      packet.push_back(v >> 8);
    }
    model->writePort("add", packet);
    writeBit("en", true);
    state = State::WaitDone;
    break;
  }

  case State::WaitDone:
    if (readBit("done")) {
      checkAccum();
      writeBit("en", false);
      opsDone++;
      state = State::WaitClear;
    }
    break;

  case State::WaitClear:
    if (!readBit("done"))
      state = State::Issue;
    break;
  }
  cycle++;

  if (opsDone == numOps) {
    primaryComponentOKToEndSim();
    return true;
  }
  return false;
}

} // namespace SST::VerilatorSST

// EOF
