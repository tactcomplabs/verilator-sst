//
// _AccumDriverDirect_h_
//
// Copyright (C) 2017-2026 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _ACCUMDRIVERDIRECT_H_
#define _ACCUMDRIVERDIRECT_H_

#include <array>
#include <cstdint>
#include <random>
#include <string>
#include <vector>

#include "SST.h"
#include "verilatorSSTAPI.h"

namespace SST::VerilatorSST {

/// AccumDriverDirect: owns the verilated Accum model (Direct build) in its
/// "model" subcomponent slot and talks to it with plain method calls. There
/// are no links and no host component. The model clocks itself.
///
/// Because the model's clock and this component's clock are separate clock
/// handlers, the relative order of their edges at a given timestamp isn't
/// something to depend on. So the driver uses the model's own `done`
/// handshake instead of counting cycles.
class AccumDriverDirect : public SST::Component {
public:
  AccumDriverDirect(SST::ComponentId_t id, const SST::Params &params);
  ~AccumDriverDirect() = default;

  void init(unsigned int phase) override;
  void setup() override;
  void finish() override;

  bool clock(SST::Cycle_t cycle);

  // clang-format off
  SST_ELI_REGISTER_COMPONENT(
    AccumDriverDirect,
    "accumdriverdirect",
    "AccumDriverDirect",
    SST_ELI_ELEMENT_VERSION(1, 0, 0),
    "Example owner of the verilated Accum model (Direct interface)",
    COMPONENT_CATEGORY_UNCATEGORIZED
  )

  SST_ELI_DOCUMENT_PARAMS(
    {"verbose",   "Sets the verbosity",                    "0"},
    {"clockFreq", "Driver clock frequency",                "1GHz"},
    {"numOps",    "Number of add transactions to perform", "8"},
    {"seed",      "Seed for the random operands",          "1"},
  )

  SST_ELI_DOCUMENT_PORTS()

  SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS(
    {"model", "Verilator Subcomponent Model", "SST::VerilatorSST::VerilatorSSTBase"},
  )
  // clang-format on

private:
  static constexpr unsigned Lanes = 4; ///< depth of add[] / accum[]

  enum class State { Reset, Issue, WaitDone, WaitClear };

  void writeBit(const std::string &port, bool v) {
    model->writePort(port, {uint8_t(v)});
  }
  bool readBit(const std::string &port) {
    const auto b = model->readPort(port);
    return !b.empty() && b[0] != 0;
  }
  void checkAccum();

  SST::Output out;
  VerilatorSSTBase *model = nullptr;

  uint64_t numOps;
  uint64_t cycle = 0;
  uint64_t opsDone = 0;
  uint64_t errors = 0;
  State state = State::Reset;
  std::mt19937 rng;
  std::array<uint32_t, Lanes> expected{}; ///< reference model of accum[]
};

} // namespace SST::VerilatorSST

#endif
