//
// _AccumLinksAdapter_h_
//
// Copyright (C) 2017-2026 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _ACCUMLINKSADAPTER_H_
#define _ACCUMLINKSADAPTER_H_

#include <deque>
#include <string>
#include <vector>

#include "AccumAccelAPI.h"
#include "verilatorSSTAPI.h"

namespace SST::VerilatorSST {

/// AccumLinksAdapter: implements AccumAccelAPI over the *Links* build of the
/// verilated Accum model. The model lives in a separate
/// verilatorcomponent.VerilatorComponent; this adapter drives it with
/// PortEvents over six links.
///
/// The link endpoints are ports of this adapter: the config connects
/// (adapter, "clk") <-> (model, "clk"), and so on. The CPU itself never sees
/// them and needs no changes. By default the Links build does not clock
/// itself, so this adapter also generates the RTL clock: one full clock cycle
/// (rising, then falling edge) per adapter clock tick. With selfClock=1 the
/// model clocks itself instead (set selfClock=1 on the model too): the adapter
/// leaves the "clk" port unconnected and only runs the en/done handshake.
class AccumLinksAdapter : public AccumAccelAPI {
public:
  AccumLinksAdapter(SST::ComponentId_t id, const SST::Params &params);
  ~AccumLinksAdapter() override = default;

  void submit(const Operands &ops, Callback done) override;
  size_t outstanding() const override { return queue.size(); }

  bool clock(SST::Cycle_t cycle);

  // clang-format off
  SST_ELI_REGISTER_SUBCOMPONENT(
    AccumLinksAdapter,
    "accumaccel",
    "AccumLinksAdapter",
    SST_ELI_ELEMENT_VERSION(1, 0, 0),
    "Accum accelerator adapter over the verilated Accum model (Links)",
    SST::VerilatorSST::AccumAccelAPI
  )

  SST_ELI_DOCUMENT_PARAMS(
    {"verbose",   "Sets the verbosity",                  "0"},
    {"clockFreq", "Rate of one RTL clock cycle per tick", "1GHz"},
    {"selfClock", "The model clocks itself; do not drive or connect clk", "false"},
  )

  // Same names as the verilog ports
  SST_ELI_DOCUMENT_PORTS(
    {"clk",     "Write the verilated clock",  {"SST::VerilatorSST::PortEvent"}},
    {"reset_l", "Write the active-low reset", {"SST::VerilatorSST::PortEvent"}},
    {"en",      "Write the add enable",       {"SST::VerilatorSST::PortEvent"}},
    {"add",     "Write 4 x 16b operands",     {"SST::VerilatorSST::PortEvent"}},
    {"accum",   "Read 4 x 32b accumulators",  {"SST::VerilatorSST::PortEvent"}},
    {"done",    "Read the done flag",         {"SST::VerilatorSST::PortEvent"}},
  )

  SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS()
  // clang-format on

private:
  enum class State { Reset, Idle, WaitDone, WaitAccum, WaitClear };

  struct Request {
    Operands ops;
    Callback done;
  };

  void write(SST::Link *link, const std::vector<uint8_t> &data) {
    link->send(new PortEvent(data));
  }
  void writeBit(SST::Link *link, bool v) { write(link, {uint8_t(v)}); }
  void read(SST::Link *link) { link->send(new PortEvent()); }

  // Read responses arrive asynchronously, between clock ticks.
  void handleDone(SST::Event *ev);
  void handleAccum(SST::Event *ev);

  SST::Output out;
  bool selfClock = false;
  SST::Link *linkClk, *linkResetL, *linkEn, *linkAdd, *linkAccum, *linkDone;
  State state = State::Reset;
  bool readPending = false; ///< a read request is outstanding on `done`
  uint64_t cycle = 0;
  std::deque<Request> queue; ///< front() is the request in flight, if any
};

} // namespace SST::VerilatorSST

#endif
