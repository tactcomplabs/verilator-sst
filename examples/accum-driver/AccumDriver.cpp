//
// _AccumDriver_cpp_
//
// Copyright (C) 2017-2026 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "AccumDriver.h"

namespace SST::VerilatorSST {

AccumDriver::AccumDriver(SST::ComponentId_t id, const SST::Params &params)
    : SST::Component(id) {
  out.init("AccumDriver[" + getName() + ":@p:@t]: ",
           params.find<int>("verbose", 0), 0, SST::Output::STDOUT);
  numOps = params.find<uint64_t>("numOps", 8);
  rng.seed(params.find<uint32_t>("seed", 1));

  // Write-only ports: no handler needed (nothing comes back on these).
  linkClk = configureLink("clk");
  linkResetL = configureLink("reset_l");
  linkEn = configureLink("en");
  linkAdd = configureLink("add");
  // Read ports: the model replies to a READ PortEvent on the same link.
  linkAccum = configureLink(
      "accum",
      new Event::Handler<AccumDriver, &AccumDriver::handleAccum>(this));
  linkDone = configureLink(
      "done", new Event::Handler<AccumDriver, &AccumDriver::handleDone>(this));
  for (auto *l : {linkClk, linkResetL, linkEn, linkAdd, linkAccum, linkDone}) {
    if (!l)
      out.fatal(CALL_INFO, -1,
                "all six Accum ports must be connected in the config\n");
  }

  registerClock(params.find<std::string>("clockFreq", "1GHz"),
                new Clock::Handler<AccumDriver, &AccumDriver::clock>(this));

  // The sim must not end until we've checked every transaction.
  registerAsPrimaryComponent();
  primaryComponentDoNotEndSim();
}

// The model (a subcomponent of the host) gets its own init() call from the
// host component, so nothing to do here.
void AccumDriver::init(unsigned int) {}
void AccumDriver::setup() {}

void AccumDriver::finish() {
  const bool ok = errors == 0 && checks == numOps && pendingAccum.empty() &&
                  pendingDone.empty();
  out.output("AccumDriver: %s (%" PRIu64 "/%" PRIu64 " checks, %" PRIu64
             " errors)\n",
             ok ? "PASSED" : "FAILED", checks, numOps, errors);
}

void AccumDriver::write(SST::Link *link, const std::vector<uint8_t> &data) {
  link->send(new PortEvent(data));
}

void AccumDriver::read(SST::Link *link) { link->send(new PortEvent()); }

bool AccumDriver::clock(SST::Cycle_t) {
  // One driver cycle = one full verilated clock cycle. The subcomponent
  // evals on every write, so the order of sends below is the order the RTL
  // sees them: rising edge, then any input changes / reads, then falling edge.
  //
  // Schedule (c = cycle):
  //   c=0  en=0 reset_l=1    c=1  reset_l=0 (async reset)    c=3  reset_l=1
  //   then repeating 3-cycle transactions starting at c=4:
  //     t+0: drive add[], en=1
  //     t+1: edge captured the add -> read accum[]/done, drive en=0
  //     t+2: edge clears done
  const uint64_t c = cycle++;

  writeBit(linkClk, true);

  if (c == 0) {
    writeBit(linkEn, false);
    writeBit(linkResetL, true);
  } else if (c == 1) {
    writeBit(linkResetL, false);
  } else if (c == 3) {
    writeBit(linkResetL, true);
  } else if (c >= 4 && opsIssued < numOps) {
    switch ((c - 4) % 3) {
    case 0: { // drive operands
      std::vector<uint8_t> packet;
      for (unsigned i = 0; i < Lanes; i++) {
        const uint16_t v = rng() & 0xffff;
        expected[i] += v;
        packet.push_back(v & 0xff);
        packet.push_back(v >> 8);
      }
      write(linkAdd, packet);
      writeBit(linkEn, true);
      break;
    }
    case 1: // check result, deassert en
      pendingAccum.push(expected);
      pendingDone.push(true);
      read(linkAccum);
      read(linkDone);
      writeBit(linkEn, false);
      opsIssued++;
      break;
    default:
      break;
    }
  }

  writeBit(linkClk, false);

  // Finished issuing; give the last reads one more cycle to come back.
  if (opsIssued == numOps && c >= 4 + 3 * numOps) {
    primaryComponentOKToEndSim();
    return true;
  }
  return false;
}

void AccumDriver::handleAccum(SST::Event *ev) {
  auto *pe = static_cast<PortEvent *>(ev);
  const auto &bytes = pe->getPacket();
  const auto want = pendingAccum.front();
  pendingAccum.pop();
  if (bytes.size() != 4 * Lanes) {
    out.fatal(CALL_INFO, -1, "accum read returned %zu bytes, expected %u\n",
              bytes.size(), 4 * Lanes);
  }
  for (unsigned i = 0; i < Lanes; i++) {
    uint32_t got = 0;
    for (unsigned b = 0; b < 4; b++)
      got |= uint32_t(bytes[i * 4 + b]) << (8 * b);
    out.verbose(CALL_INFO, 2, 0, "accum[%u] = 0x%08x (want 0x%08x)\n", i, got,
                want[i]);
    if (got != want[i]) {
      errors++;
      out.output("accum[%u] mismatch: got 0x%08x want 0x%08x\n", i, got,
                 want[i]);
    }
  }
  checks++;
  delete pe;
}

void AccumDriver::handleDone(SST::Event *ev) {
  auto *pe = static_cast<PortEvent *>(ev);
  const bool want = pendingDone.front();
  pendingDone.pop();
  const bool got = !pe->getPacket().empty() && pe->getPacket()[0] != 0;
  if (got != want) {
    errors++;
    out.output("done mismatch: got %d want %d\n", got, want);
  }
  delete pe;
}

} // namespace SST::VerilatorSST

// EOF
