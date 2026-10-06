//
// _AccumLinkClient_cpp_
//
// Copyright (C) 2017-2026 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "AccumLinkClient.h"

namespace SST::VerilatorSST {

AccumLinkClient::AccumLinkClient(SST::ComponentId_t id,
                                 const SST::Params &params)
    : AccumAccelAPI(id) {
  out.init("AccumLinkClient[" + getName() + ":@p:@t]: ",
           params.find<int>("verbose", 0), 0, SST::Output::STDOUT);
  link = configureLink(
      "link", new Event::Handler<AccumLinkClient, &AccumLinkClient::handleResp>(
                  this));
  if (!link)
    out.fatal(CALL_INFO, -1, "\"link\" is not connected in the config\n");
}

void AccumLinkClient::submit(const Operands &ops, Callback done) {
  const uint64_t id = nextId++;
  pending.emplace(id, std::move(done));
  link->send(new AccumReq(id, ops));
}

void AccumLinkClient::handleResp(SST::Event *ev) {
  auto *resp = static_cast<AccumResp *>(ev);
  auto it = pending.find(resp->id);
  if (it == pending.end())
    out.fatal(CALL_INFO, -1, "response for unknown request id %" PRIu64 "\n",
              resp->id);
  const auto sums = resp->sums;
  // erase before the callback: the client may submit() from inside it
  Callback cb = std::move(it->second);
  pending.erase(it);
  delete resp;
  cb(sums);
}

} // namespace SST::VerilatorSST

// EOF
