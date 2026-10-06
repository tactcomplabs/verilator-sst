//
// _StubCpu_cpp_
//
// Copyright (C) 2017-2026 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "StubCpu.h"

namespace SST::VerilatorSST {

StubCpu::StubCpu(SST::ComponentId_t id, const SST::Params &params)
    : SST::Component(id) {
  out.init("StubCpu[" + getName() + ":@p:@t]: ", params.find<int>("verbose", 0),
           0, SST::Output::STDOUT);
  numOps = params.find<uint64_t>("numOps", 12);
  burst = params.find<uint64_t>("burst", 4);
  period = params.find<uint64_t>("period", 40);
  exact = params.find<bool>("exact", true);
  rng.seed(params.find<uint32_t>("seed", 1));

  accel = loadUserSubComponent<AccumAccelAPI>("accel");
  if (!accel)
    out.fatal(CALL_INFO, -1, "no subcomponent loaded in slot \"accel\"\n");

  registerClock(params.find<std::string>("clockFreq", "2GHz"),
                new Clock::Handler<StubCpu, &StubCpu::clock>(this));

  registerAsPrimaryComponent();
  primaryComponentDoNotEndSim();
}

void StubCpu::init(unsigned int phase) { accel->init(phase); }

void StubCpu::finish() {
  const bool ok = errors == 0 && completed == numOps;
  out.output("StubCpu: %s (%" PRIu64 "/%" PRIu64 " results, %" PRIu64
             " errors, latency %" PRIu64 "-%" PRIu64
             " cpu cycles, max queue %zu)\n",
             ok ? "PASSED" : "FAILED", completed, numOps, errors,
             completed ? minLat : 0, maxLat, maxQueue);
}

void StubCpu::issue() {
  AccumAccelAPI::Operands ops;
  for (auto &v : ops) {
    v = rng() & 0xffff;
  }
  for (unsigned i = 0; i < ops.size(); i++)
    expected[i] += ops[i];

  const uint64_t submitted = cycle;
  const auto want = expected; // totals this request should produce
  const uint64_t n = issued++;

  // The callback fires from inside the accelerator's clock handler.
  accel->submit(ops, [this, n, submitted, want](const AccumAccelAPI::Sums &got) {
    const uint64_t lat = cycle - submitted;
    minLat = std::min(minLat, lat);
    maxLat = std::max(maxLat, lat);
    for (unsigned i = 0; i < got.size(); i++) {
      // shared accelerator: other clients only ever add, so our own totals
      // are a lower bound (no wraparound at these sizes)
      const bool bad = exact ? got[i] != want[i] : got[i] < want[i];
      if (bad) {
        errors++;
        out.output("req %" PRIu64 " lane %u: got 0x%08x want %s0x%08x\n", n, i,
                   got[i], exact ? "" : ">= ", want[i]);
      }
    }
    out.verbose(CALL_INFO, 2, 0, "req %" PRIu64 " done after %" PRIu64
                                 " cpu cycles\n",
                n, lat);
    completed++;
    if (completed == numOps)
      primaryComponentOKToEndSim();
  });
  maxQueue = std::max(maxQueue, accel->outstanding());
}

bool StubCpu::clock(SST::Cycle_t) {
  // "Compute" for `period` cycles, then dump a burst of work on the accelerator
  // and carry on; results come back through the callback.
  if (issued < numOps && cycle % period == 0) {
    for (uint64_t i = 0; i < burst && issued < numOps; i++)
      issue();
  }
  cycle++;
  return completed == numOps;
}

} // namespace SST::VerilatorSST

// EOF
