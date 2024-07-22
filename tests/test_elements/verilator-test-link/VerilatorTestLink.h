//
// _VerilatorTestLink_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _VERILATOR_TEST_LINK_H_
#define _VERILATOR_TEST_LINK_H_

// -- Standard Headers
#include <list>
#include <memory>
#include <queue>
#include <random>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <tuple>
#include <utility>
#include <vector>

// -- SST Headers
#include "SST.h"

// -- Verilator SST Headers
#include "verilatorSSTSubcomponent.h"

namespace SST::VerilatorSST {

class VerilatorTestLink : public SST::Component {
  /// VerilatorTestLink: constructor
  VerilatorTestLink(SST::ComponentId_t id, const SST:Params& params);

  /// VerilatorTestLink: destructor
  ~VerilatorTestLink();

  /// VerilatorTestLink: setup functions
  void setup();

  /// VerilatorTestLink: finish function
  void finish();

  /// VerilatorTestLink: init function
  void init(unsigned int phase);

  /// VerilatorTestLink: clock function
  bool clock(SST::Cycle_t currentCycle);

  // -------------------------------------------------------
  // VerilatorTestLink Component Registration Data
  // -------------------------------------------------------
  SST_ELI_REGISTER_COMPONENT(
    VerilatorTestLink,    // component class
    "verilatortestlink",  // component library
    "VerilatorTestlink",  // component name
    SST_ELI_ELEMENT_VERSION( 1, 0, 0 ),
    "VerilatorSST Link Infrastructure Test Component",
    COMPONENT_CATEGORY_UNCATEGORIZED
  )

  // -------------------------------------------------------
  // VerilatorTestLink Component Parameter Data
  // -------------------------------------------------------
  // clang-format off
  SST_ELI_DOCUMENT_PARAMS(
    {"verbose",     "Sets the verbosity",       "0"},
    {"clockFreq",   "Clock frequency",          "1GHz"},
    {"numCycles",   "Number of cycles to exec", "1000"},
  )

  // -------------------------------------------------------
  // VerilatorTestLink Port Parameter Data
  // -------------------------------------------------------
  SST_ELI_DOCUMENT_PORTS(
    {"port%(num_ports)d",
      "Ports which connect to tested verilated subcomponents.",
      {"SST::VerilatorSST::PortEvent", ""}
    }
  )

  // -------------------------------------------------------
  // VerilatorTestLink SubComponent Parameter Data
  // -------------------------------------------------------
  SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS(
    {"model", "Verilator Subcomponent Model",   "SST::VerilatorSST::VerilatorSSTBase"},
  )

private:
  SST::Output output;                           ///< VerilatorTestLink: SST output
  bool primaryComponent;                        ///< VerilatorTestLink: registers as the primary component
  uint64_t NumCycles;                           ///< VerilatorTestLink: number of cycles to execute
  SST::VerilatorSST::VerilatorSSTBase *model;   ///< VerilatorTestLink: subcomponent model

};  // class VerilatorTestLink
};  // namespace SST::VerilatorSST

#endif  // _VERILATOR_TEST_LINK_H_
