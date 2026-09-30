//
// _AccumAccelAPI_h_
//
// Copyright (C) 2017-2026 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _ACCUMACCELAPI_H_
#define _ACCUMACCELAPI_H_

#include <array>
#include <cstdint>
#include <functional>

#include "SST.h"

namespace SST::VerilatorSST {

/// AccumAccelAPI: what a CPU (or any client component) sees of the Accum
/// accelerator. Nothing here mentions verilator, ports, or clocks, so the
/// implementation behind it can be swapped (RTL, a functional model, ...).
///
/// Semantics: the accelerator keeps four running 32-bit totals. submit()
/// adds four 16-bit operands into them; the callback receives the totals
/// *after* that request was applied. Requests are served in FIFO order.
class AccumAccelAPI : public SST::SubComponent {
public:
  SST_ELI_REGISTER_SUBCOMPONENT_API(SST::VerilatorSST::AccumAccelAPI)

  static constexpr unsigned Lanes = 4;
  using Operands = std::array<uint16_t, Lanes>;
  using Sums = std::array<uint32_t, Lanes>;
  using Callback = std::function<void(const Sums &)>;

  explicit AccumAccelAPI(SST::ComponentId_t id) : SST::SubComponent(id) {}
  ~AccumAccelAPI() override = default;

  /// Queue a request; never blocks. `done` runs when the result is ready.
  virtual void submit(const Operands &ops, Callback done) = 0;

  /// Number of requests queued or in flight
  virtual size_t outstanding() const = 0;
};

} // namespace SST::VerilatorSST

#endif
