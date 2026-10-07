//
// _AccumEngine_cpp_
//
// Copyright (C) 2017-2026 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "AccumEngine.h"

namespace SST::VerilatorSST {

void AccumEngine::submit(const Operands &ops, Callback done) {
  queue.push_back({ops, std::move(done)});
}

void AccumEngine::step() {
  switch (state) {
  case State::Reset:
    // reset_l 1 -> 0 -> 1 so the RTL sees a falling edge regardless of its
    // random power-up value. Requests submitted meanwhile just queue.
    if (cycle == 0) {
      writeBit("en", false);
      writeBit("reset_l", true);
    } else if (cycle == 1) {
      writeBit("reset_l", false);
    } else {
      writeBit("reset_l", true);
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
      model->writePort("add", packet);
      writeBit("en", true);
      state = State::WaitDone;
    }
    break;

  case State::WaitDone:
    if (readBit("done")) {
      const auto bytes = model->readPort("accum");
      Sums sums{};
      for (unsigned i = 0; i < Lanes; i++)
        for (unsigned b = 0; b < 4; b++)
          sums[i] |= uint32_t(bytes[i * 4 + b]) << (8 * b);
      writeBit("en", false);

      // pop before the callback: the client may submit() from inside it
      Callback cb = std::move(queue.front().done);
      queue.pop_front();
      completed++;
      state = State::WaitClear;
      cb(sums);
    }
    break;

  case State::WaitClear:
    if (!readBit("done"))
      state = State::Idle;
    break;
  }
  cycle++;
}

} // namespace SST::VerilatorSST

// EOF
