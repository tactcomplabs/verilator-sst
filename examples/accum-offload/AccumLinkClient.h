//
// _AccumLinkClient_h_
//
// Copyright (C) 2017-2026 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _ACCUMLINKCLIENT_H_
#define _ACCUMLINKCLIENT_H_

#include <string>
#include <unordered_map>

#include "AccumEvents.h"

namespace SST::VerilatorSST {

/// AccumLinkClient: AccumAccelAPI over an SST link to an AccumAccelComponent.
/// Drop it in a client's "accel" slot in place of AccumAdapter; the client
/// (e.g. StubCpu) is unchanged. The link's latency models the trip to the
/// accelerator and back.
class AccumLinkClient : public AccumAccelAPI {
public:
  AccumLinkClient(SST::ComponentId_t id, const SST::Params &params);
  ~AccumLinkClient() override = default;

  void submit(const Operands &ops, Callback done) override;
  size_t outstanding() const override { return pending.size(); }

  // clang-format off
  SST_ELI_REGISTER_SUBCOMPONENT(
    AccumLinkClient,
    "accumaccel",
    "AccumLinkClient",
    SST_ELI_ELEMENT_VERSION(1, 0, 0),
    "AccumAccelAPI client that talks to an AccumAccelComponent over a link",
    SST::VerilatorSST::AccumAccelAPI
  )

  SST_ELI_DOCUMENT_PARAMS(
    {"verbose", "Sets the verbosity", "0"},
  )

  SST_ELI_DOCUMENT_PORTS(
    {"link", "Link to an AccumAccelComponent client port",
      {"SST::VerilatorSST::AccumReq", "SST::VerilatorSST::AccumResp"}},
  )

  SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS()
  // clang-format on

private:
  void handleResp(SST::Event *ev);

  SST::Output out;
  SST::Link *link = nullptr;
  uint64_t nextId = 0;
  /// callbacks can't cross a link, so keep them here keyed by request id
  std::unordered_map<uint64_t, Callback> pending;
};

} // namespace SST::VerilatorSST

#endif
