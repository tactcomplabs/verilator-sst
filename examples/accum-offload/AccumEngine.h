//
// _AccumEngine_h_
//
// Copyright (C) 2017-2026 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _ACCUMENGINE_H_
#define _ACCUMENGINE_H_

#include <deque>
#include <string>
#include <vector>

#include "AccumAccelAPI.h"
#include "verilatorSSTAPI.h"

namespace SST::VerilatorSST {

/// AccumEngine: everything needed to run requests through the Direct build of
/// the verilated Accum model: a FIFO of requests plus the reset and en/done
/// handshake. It is plain C++ (not an SST element) and is shared by every
/// place that hosts the model: AccumAdapter (subcomponent) and
/// AccumAccelComponent (link-connected component).
///
/// The host calls step() once per tick of its own clock.
class AccumEngine {
public:
  using Operands = AccumAccelAPI::Operands;
  using Sums = AccumAccelAPI::Sums;
  using Callback = AccumAccelAPI::Callback;
  static constexpr unsigned Lanes = AccumAccelAPI::Lanes;

  AccumEngine(SST::Output &out, VerilatorSSTBase *model)
      : out(out), model(model) {}

  /// forward SST init() to the model (SST only calls init on the parent)
  void init(unsigned int phase) { model->init(phase); }

  /// Queue a request; `done` runs from step() when its result is ready.
  void submit(const Operands &ops, Callback done);

  /// Requests queued or in flight
  size_t outstanding() const { return queue.size(); }

  /// Advance the handshake by one tick
  void step();

  uint64_t completedCount() const { return completed; }

private:
  enum class State { Reset, Idle, WaitDone, WaitClear };

  struct Request {
    Operands ops;
    Callback done;
  };

  void writeBit(const std::string &port, bool v) {
    model->writePort(port, {uint8_t(v)});
  }
  bool readBit(const std::string &port) {
    const auto b = model->readPort(port);
    return !b.empty() && b[0] != 0;
  }

  SST::Output &out;
  VerilatorSSTBase *model;
  State state = State::Reset;
  uint64_t cycle = 0;
  uint64_t completed = 0;
  std::deque<Request> queue; ///< front() is the request in flight, if any
};

} // namespace SST::VerilatorSST

#endif
