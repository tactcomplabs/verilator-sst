//
// _AccumLinksAdapter_cpp_
//
// Copyright (C) 2017-2026 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "AccumLinksAdapter.h"

namespace SST::VerilatorSST {

AccumLinksAdapter::AccumLinksAdapter(SST::ComponentId_t id,
                                     const SST::Params &params)
    : AccumAccelAPI(id) {
  out.init("AccumLinksAdapter[" + getName() + ":@p:@t]: ",
           params.find<int>("verbose", 0), 0, SST::Output::STDOUT);

  // Write-only ports need no handler; read ports get the model's reply.
  selfClock = params.find<bool>("selfClock", false);
  // not needed (and left unconnected in the config) when the model clocks itself
  linkClk = selfClock ? nullptr : configureLink("clk");
  linkResetL = configureLink("reset_l");
  linkEn = configureLink("en");
  linkAdd = configureLink("add");
  linkAccum = configureLink(
      "accum", new Event::Handler<AccumLinksAdapter,
                                  &AccumLinksAdapter::handleAccum>(this));
  linkDone = configureLink(
      "done", new Event::Handler<AccumLinksAdapter,
                                 &AccumLinksAdapter::handleDone>(this));
  for (auto *l : {linkResetL, linkEn, linkAdd, linkAccum, linkDone}) {
    if (!l)
      out.fatal(CALL_INFO, -1,
                "reset_l, en, add, accum and done must be connected in the "
                "config\n");
  }
  if (!selfClock && !linkClk)
    out.fatal(CALL_INFO, -1, "clk must be connected unless selfClock is set\n");

  registerClock(params.find<std::string>("clockFreq", "1GHz"),
                new Clock::Handler<AccumLinksAdapter,
                                   &AccumLinksAdapter::clock>(this));
}

void AccumLinksAdapter::submit(const Operands &ops, Callback done) {
  queue.push_back({ops, std::move(done)});
}

bool AccumLinksAdapter::clock(SST::Cycle_t) {
  // One adapter tick == one RTL clock cycle. Everything sent between the
  // rising and falling edge is seen by the RTL after the rising edge.
  if (!selfClock)
    writeBit(linkClk, true);

  switch (state) {
  case State::Reset:
    if (cycle == 0) {
      writeBit(linkEn, false);
      writeBit(linkResetL, true);
    } else if (cycle == 1) {
      writeBit(linkResetL, false);
    } else {
      writeBit(linkResetL, true);
      state = State::Idle;
    }
    break;

  case State::Idle:
    if (!queue.empty()) {
      std::vector<uint8_t> packet;
      for (uint16_t v : queue.front().ops) {
        packet.push_back(v & 0xff);
        packet.push_back(v >> 8);
      }
      write(linkAdd, packet);
      writeBit(linkEn, true);
      state = State::WaitDone;
    }
    break;

  case State::WaitDone:
  case State::WaitClear:
    // poll the RTL's done flag; the reply drives the state machine
    if (!readPending) {
      readPending = true;
      read(linkDone);
    }
    break;

  case State::WaitAccum:
    break; // waiting for the accum read issued by handleDone
  }

  if (!selfClock)
    writeBit(linkClk, false);
  cycle++;
  return false;
}

void AccumLinksAdapter::handleDone(SST::Event *ev) {
  auto *pe = static_cast<PortEvent *>(ev);
  const bool done = !pe->getPacket().empty() && pe->getPacket()[0] != 0;
  delete pe;
  readPending = false;

  if (state == State::WaitDone && done) {
    read(linkAccum);
    state = State::WaitAccum;
  } else if (state == State::WaitClear && !done) {
    state = State::Idle;
  }
}

void AccumLinksAdapter::handleAccum(SST::Event *ev) {
  auto *pe = static_cast<PortEvent *>(ev);
  const auto bytes = pe->getPacket();
  delete pe;
  if (bytes.size() != 4 * Lanes)
    out.fatal(CALL_INFO, -1, "accum read returned %zu bytes, expected %u\n",
              bytes.size(), 4 * Lanes);

  Sums sums{};
  for (unsigned i = 0; i < Lanes; i++)
    for (unsigned b = 0; b < 4; b++)
      sums[i] |= uint32_t(bytes[i * 4 + b]) << (8 * b);
  writeBit(linkEn, false);

  // pop before the callback: the client may submit() from inside it
  Callback cb = std::move(queue.front().done);
  queue.pop_front();
  state = State::WaitClear;
  cb(sums);
}

} // namespace SST::VerilatorSST

// EOF
