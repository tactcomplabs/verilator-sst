//
// _AccumAdapter_h_
//
// Copyright (C) 2017-2026 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _ACCUMADAPTER_H_
#define _ACCUMADAPTER_H_

#include <deque>
#include <string>

#include "AccumAccelAPI.h"
#include "verilatorSSTAPI.h"

namespace SST::VerilatorSST {

/// AccumAdapter: implements AccumAccelAPI on top of the Direct build of the
/// verilated Accum model, which it owns in its own "model" slot. It hides the
/// RTL protocol (reset, en/done handshake, byte packing) from the client.
class AccumAdapter : public AccumAccelAPI {
public:
  AccumAdapter(SST::ComponentId_t id, const SST::Params &params);
  ~AccumAdapter() override = default;

  void init(unsigned int phase) override;
  void finish() override;

  void submit(const Operands &ops, Callback done) override;
  size_t outstanding() const override { return queue.size(); }

  bool clock(SST::Cycle_t cycle);

  // clang-format off
  SST_ELI_REGISTER_SUBCOMPONENT(
    AccumAdapter,
    "accumaccel",
    "AccumAdapter",
    SST_ELI_ELEMENT_VERSION(1, 0, 0),
    "Accum accelerator adapter over the verilated Accum model (Direct)",
    SST::VerilatorSST::AccumAccelAPI
  )

  SST_ELI_DOCUMENT_PARAMS(
    {"verbose",   "Sets the verbosity",                "0"},
    {"clockFreq", "Adapter (handshake) clock frequency", "1GHz"},
  )

  SST_ELI_DOCUMENT_PORTS()

  SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS(
    {"model", "Verilator Subcomponent Model", "SST::VerilatorSST::VerilatorSSTBase"},
  )
  // clang-format on

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

  SST::Output out;
  VerilatorSSTBase *model = nullptr;
  State state = State::Reset;
  uint64_t cycle = 0;
  uint64_t completed = 0;
  std::deque<Request> queue; ///< front() is the request in flight, if any
};

} // namespace SST::VerilatorSST

#endif
