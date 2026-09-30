//
// _AccumAccelComponent_cpp_
//
// Copyright (C) 2017-2026 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "AccumAccelComponent.h"

namespace SST::VerilatorSST {

AccumAccelComponent::AccumAccelComponent(SST::ComponentId_t id,
                                         const SST::Params &params)
    : SST::Component(id) {
  out.init("AccumAccel[" + getName() + ":@p:@t]: ",
           params.find<int>("verbose", 0), 0, SST::Output::STDOUT);

  model = loadUserSubComponent<VerilatorSSTBase>("model");
  if (!model)
    out.fatal(CALL_INFO, -1, "no subcomponent loaded in slot \"model\"\n");
  engine = std::make_unique<AccumEngine>(out, model);

  const unsigned n = params.find<unsigned>("num_clients", 1);
  for (unsigned i = 0; i < n; i++) {
    auto *l = configureLink(
        "client" + std::to_string(i),
        new Event::Handler<AccumAccelComponent, &AccumAccelComponent::handleReq,
                           unsigned>(this, i));
    if (!l)
      out.fatal(CALL_INFO, -1, "client%u is not connected in the config\n", i);
    links.push_back(l);
  }
  served.assign(n, 0);

  registerClock(params.find<std::string>("clockFreq", "1GHz"),
                new Clock::Handler<AccumAccelComponent,
                                   &AccumAccelComponent::clock>(this));
}

// Forward to the model; SST only calls init() on the component itself.
void AccumAccelComponent::init(unsigned int phase) { engine->init(phase); }

void AccumAccelComponent::finish() {
  for (unsigned i = 0; i < served.size(); i++)
    out.verbose(CALL_INFO, 1, 0, "client%u: %" PRIu64 " requests served\n", i,
                served[i]);
}

bool AccumAccelComponent::clock(SST::Cycle_t) {
  engine->step();
  return false;
}

void AccumAccelComponent::handleReq(SST::Event *ev, unsigned client) {
  auto *req = static_cast<AccumReq *>(ev);
  const uint64_t id = req->id;
  out.verbose(CALL_INFO, 2, 0, "client%u request %" PRIu64 " (queue %zu)\n",
              client, id, engine->outstanding());
  // The response goes back on the link the request came in on.
  engine->submit(req->ops, [this, client, id](const AccumEngine::Sums &sums) {
    served[client]++;
    links[client]->send(new AccumResp(id, sums));
  });
  delete req;
}

} // namespace SST::VerilatorSST

// EOF
