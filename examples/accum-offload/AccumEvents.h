//
// _AccumEvents_h_
//
// Copyright (C) 2017-2026 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _ACCUMEVENTS_H_
#define _ACCUMEVENTS_H_

#include "AccumAccelAPI.h"

namespace SST::VerilatorSST {

/// AccumReq: client -> accelerator. `id` is chosen by the client and echoed
/// back, since a callback cannot travel over a link.
class AccumReq : public SST::Event {
public:
  AccumReq() : Event() {} // needed by serialization
  AccumReq(uint64_t id, const AccumAccelAPI::Operands &ops)
      : Event(), id(id), ops(ops) {}

  Event *clone() override { return new AccumReq(*this); }

  uint64_t id = 0;
  AccumAccelAPI::Operands ops{};

  void serialize_order(SST::Core::Serialization::serializer &ser) override {
    Event::serialize_order(ser);
    SST_SER(id);
    SST_SER(ops);
  }
  ImplementSerializable(SST::VerilatorSST::AccumReq);
};

/// AccumResp: accelerator -> client; the totals after request `id` applied.
class AccumResp : public SST::Event {
public:
  AccumResp() : Event() {}
  AccumResp(uint64_t id, const AccumAccelAPI::Sums &sums)
      : Event(), id(id), sums(sums) {}

  Event *clone() override { return new AccumResp(*this); }

  uint64_t id = 0;
  AccumAccelAPI::Sums sums{};

  void serialize_order(SST::Core::Serialization::serializer &ser) override {
    Event::serialize_order(ser);
    SST_SER(id);
    SST_SER(sums);
  }
  ImplementSerializable(SST::VerilatorSST::AccumResp);
};

} // namespace SST::VerilatorSST

#endif
