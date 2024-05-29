//
// _VerilatorTestDirect_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _VERILATOR_TEST_DIRECT_H_
#define _VERILATOR_TEST_DIRECT_H_

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

class VerilatorTestDirect : public SST::Component {
public:
  /// VerilatorTestDirect: constuctor
  VerilatorTestDirect(SST::ComponentId_t id, const SST::Params& params);

  /// VerilatorTestDirect: destructor
  ~VerilatorTestDirect();

  /// VerilatorTestDirect: setup function
  void setup();

  /// VerilatorTestDirect: finish function
  void finish();

  /// VerilatorTestDirect: init function
  void init( unsigned int phase );

  /// VerilatorTestDirect: clock function
  bool clock(SST::Cycle_t currentCycle );

  // -------------------------------------------------------
  // VerilatorTestDirect Component Registration Data
  // -------------------------------------------------------
  SST_ELI_REGISTER_COMPONENT(
    VerilatorTestDirect,  // component class
    "verilatortestdirect",// component library
    "VerilatorTestDirect",// component name
    SST_ELI_ELEMENT_VERSION( 1, 0, 0 ),
    "VerilatorSST Direct API Test Component",
    COMPONENT_CATEGORY_UNCATEGORIZED
  )

  // -------------------------------------------------------
  // VerilatorTestDirect Component Parameter Data
  // -------------------------------------------------------
  // clang-format off
  SST_ELI_DOCUMENT_PARAMS(
    {"verbose",     "Sets the verbosity",       "0"},
    {"clock",       "Clock frequency",          "1GHz"},
    {"numCycles",   "Number of cycles to exec", "1000"},
  )

  // -------------------------------------------------------
  // VerilatorTestDirect Port Parameter Data
  // -------------------------------------------------------
  SST_ELI_DOCUMENT_PORTS(
  )

  // -------------------------------------------------------
  // VerilatorTestDirect SubComponent Parameter Data
  // -------------------------------------------------------
  SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS(
    {"model", "Verilator Subcomponent Model",   "SST::VerilatorSST::VerilatorSSTBase"},
  )
private:
  SST::Output    output;                          ///< VerilatorTestDirect: SST output

  uint64_t NumCycles;                             ///< VerilatorTestDirect: number of cycles to execute

  SST::VerilatorSST::VerilatorSSTBase *model;     ///< VerilatorTestDirect: subcomponent model

  /// generate random input data
  std::vector<uint8_t> generateData(unsigned Width);

};  // class VerilatorTestDirect

};  // namespace SST::VerilatorSST

#endif  // _VERILATOR_TEST_DIRECT_H_

// EOF
