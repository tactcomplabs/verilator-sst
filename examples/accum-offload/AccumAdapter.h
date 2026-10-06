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

#include <memory>
#include <string>

#include "AccumAccelAPI.h"
#include "AccumEngine.h"
#include "verilatorSSTAPI.h"

namespace SST::VerilatorSST {

/// AccumAdapter: implements AccumAccelAPI on top of the Direct build of the
/// verilated Accum model, which it owns in its own "model" slot. The RTL
/// protocol (reset, en/done handshake, byte packing) lives in AccumEngine.
class AccumAdapter : public AccumAccelAPI {
public:
  AccumAdapter(SST::ComponentId_t id, const SST::Params &params);
  ~AccumAdapter() override = default;

  void init(unsigned int phase) override;
  void finish() override;

  void submit(const Operands &ops, Callback done) override;
  size_t outstanding() const override { return engine->outstanding(); }

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
  SST::Output out;
  VerilatorSSTBase *model = nullptr;
  std::unique_ptr<AccumEngine> engine; ///< RTL handshake, shared with the
                                       ///< link-connected component
};

} // namespace SST::VerilatorSST

#endif
