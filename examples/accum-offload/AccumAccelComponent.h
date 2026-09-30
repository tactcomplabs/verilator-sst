//
// _AccumAccelComponent_h_
//
// Copyright (C) 2017-2026 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _ACCUMACCELCOMPONENT_H_
#define _ACCUMACCELCOMPONENT_H_

#include <memory>
#include <string>
#include <vector>

#include "AccumEngine.h"
#include "AccumEvents.h"

namespace SST::VerilatorSST {

/// AccumAccelComponent: the verilated Accum model as a standalone component
/// (think: a chiplet) that any number of clients reach over SST links. It
/// owns the Direct-build model in its "model" slot and serves requests from
/// all clients in arrival order through one AccumEngine.
///
/// The accelerator state is shared: the RTL keeps one set of running totals,
/// so a client's result includes every other client's earlier requests.
class AccumAccelComponent : public SST::Component {
public:
  AccumAccelComponent(SST::ComponentId_t id, const SST::Params &params);
  ~AccumAccelComponent() = default;

  void init(unsigned int phase) override;
  void finish() override;

  bool clock(SST::Cycle_t cycle);

  // clang-format off
  SST_ELI_REGISTER_COMPONENT(
    AccumAccelComponent,
    "accumaccel",
    "AccumAccelComponent",
    SST_ELI_ELEMENT_VERSION(1, 0, 0),
    "Verilated Accum model as a shared, link-connected accelerator",
    COMPONENT_CATEGORY_UNCATEGORIZED
  )

  SST_ELI_DOCUMENT_PARAMS(
    {"verbose",     "Sets the verbosity",                       "0"},
    {"clockFreq",   "Rate the RTL handshake is stepped",        "1GHz"},
    {"num_clients", "Number of client links (client0..N-1)",    "1"},
  )

  SST_ELI_DOCUMENT_PORTS(
    {"client%(num_clients)d", "Link to an AccumLinkClient",
      {"SST::VerilatorSST::AccumReq", "SST::VerilatorSST::AccumResp"}},
  )

  SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS(
    {"model", "Verilator Subcomponent Model", "SST::VerilatorSST::VerilatorSSTBase"},
  )
  // clang-format on

private:
  void handleReq(SST::Event *ev, unsigned client);

  SST::Output out;
  VerilatorSSTBase *model = nullptr;
  std::unique_ptr<AccumEngine> engine;
  std::vector<SST::Link *> links; ///< one per client
  std::vector<uint64_t> served;   ///< requests completed per client
};

} // namespace SST::VerilatorSST

#endif
