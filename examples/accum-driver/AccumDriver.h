//
// _AccumDriver_h_
//
// Copyright (C) 2017-2026 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _ACCUMDRIVER_H_
#define _ACCUMDRIVER_H_

#include <array>
#include <cstdint>
#include <queue>
#include <random>
#include <string>
#include <vector>

#include "SST.h"
#include "verilatorSSTAPI.h"

namespace SST::VerilatorSST {

/// AccumDriver: drives the verilated Accum model (verilatorsstAccum) over
/// SST links. It plays the role of the surrounding system: it generates the
/// clock, applies reset, feeds operands, and checks the running sums.
///
/// Wiring (see accum-driver.py): each driver port connects to the port of the
/// same name on the verilatorcomponent.VerilatorComponent that hosts the model.
class AccumDriver : public SST::Component {
public:
  AccumDriver(SST::ComponentId_t id, const SST::Params &params);
  ~AccumDriver() = default;

  void init(unsigned int phase) override;
  void setup() override;
  void finish() override;

  /// one call == one cycle of the *verilated* clock (rising then falling edge)
  bool clock(SST::Cycle_t cycle);

  // clang-format off
  SST_ELI_REGISTER_COMPONENT(
    AccumDriver,
    "accumdriver",
    "AccumDriver",
    SST_ELI_ELEMENT_VERSION(1, 0, 0),
    "Example driver for the verilated Accum model",
    COMPONENT_CATEGORY_UNCATEGORIZED
  )

  SST_ELI_DOCUMENT_PARAMS(
    {"verbose",   "Sets the verbosity",                          "0"},
    {"clockFreq", "Driver clock frequency",                      "1GHz"},
    {"numOps",    "Number of add transactions to perform",       "8"},
    {"seed",      "Seed for the random operands",                "1"},
  )

  // Port names intentionally match the verilog top-level ports.
  SST_ELI_DOCUMENT_PORTS(
    {"clk",     "Write the verilated clock",  {"SST::VerilatorSST::PortEvent"}},
    {"reset_l", "Write the active-low reset", {"SST::VerilatorSST::PortEvent"}},
    {"en",      "Write the add enable",       {"SST::VerilatorSST::PortEvent"}},
    {"add",     "Write 4 x 16b operands",     {"SST::VerilatorSST::PortEvent"}},
    {"accum",   "Read 4 x 32b accumulators",  {"SST::VerilatorSST::PortEvent"}},
    {"done",    "Read the done flag",         {"SST::VerilatorSST::PortEvent"}},
  )
  // clang-format on

private:
  static constexpr unsigned Lanes = 4; ///< depth of add[] / accum[]

  /// Send a write of `data` to the named link (little-endian bytes)
  void write(SST::Link *link, const std::vector<uint8_t> &data);
  void writeBit(SST::Link *link, bool v) { write(link, {uint8_t(v)}); }

  /// Send a read request; the response arrives in the matching handler
  void read(SST::Link *link);

  void handleAccum(SST::Event *ev);
  void handleDone(SST::Event *ev);

  SST::Output out;
  SST::Link *linkClk, *linkResetL, *linkEn, *linkAdd, *linkAccum, *linkDone;

  uint64_t numOps;
  uint64_t cycle = 0; ///< verilated-clock cycles issued so far
  uint64_t opsIssued = 0;
  uint64_t checks = 0;
  uint64_t errors = 0;
  std::mt19937 rng;

  std::array<uint32_t, Lanes> expected{}; ///< reference model of accum[]
  std::queue<std::array<uint32_t, Lanes>> pendingAccum;
  std::queue<bool> pendingDone;
};

} // namespace SST::VerilatorSST

#endif
