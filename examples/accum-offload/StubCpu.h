//
// _StubCpu_h_
//
// Copyright (C) 2017-2026 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _STUBCPU_H_
#define _STUBCPU_H_

#include <random>
#include <string>

#include "AccumAccelAPI.h"

namespace SST::VerilatorSST {

/// StubCpu: stand-in for a CPU/accelerator model that offloads work. It
/// submits bursts of requests to whatever AccumAccelAPI is in its "accel"
/// slot, and checks each result against its own running totals.
class StubCpu : public SST::Component {
public:
  StubCpu(SST::ComponentId_t id, const SST::Params &params);
  ~StubCpu() = default;

  void init(unsigned int phase) override;
  void finish() override;

  bool clock(SST::Cycle_t cycle);

  // clang-format off
  SST_ELI_REGISTER_COMPONENT(
    StubCpu,
    "accumaccel",
    "StubCpu",
    SST_ELI_ELEMENT_VERSION(1, 0, 0),
    "Stub CPU that offloads accumulate requests to an AccumAccelAPI",
    COMPONENT_CATEGORY_UNCATEGORIZED
  )

  SST_ELI_DOCUMENT_PARAMS(
    {"verbose",   "Sets the verbosity",                          "0"},
    {"clockFreq", "CPU clock frequency",                         "2GHz"},
    {"numOps",    "Total number of requests to offload",         "12"},
    {"burst",     "Requests submitted back-to-back per period",  "4"},
    {"period",    "CPU cycles between bursts",                   "40"},
    {"seed",      "Seed for the random operands",                "1"},
  )

  SST_ELI_DOCUMENT_PORTS()

  SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS(
    {"accel", "Accumulator accelerator", "SST::VerilatorSST::AccumAccelAPI"},
  )
  // clang-format on

private:
  void issue();

  SST::Output out;
  AccumAccelAPI *accel = nullptr;

  uint64_t numOps, burst, period;
  uint64_t cycle = 0;
  uint64_t issued = 0;
  uint64_t completed = 0;
  uint64_t errors = 0;
  uint64_t minLat = UINT64_MAX, maxLat = 0; ///< in CPU cycles
  size_t maxQueue = 0;
  std::mt19937 rng;
  AccumAccelAPI::Sums expected{}; ///< running totals, in submission order
};

} // namespace SST::VerilatorSST

#endif
