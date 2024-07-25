//
// _VerilatorComponent_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _VERILATOR_COMPONENT_H_
#define _VERILATOR_COMPONENT_H_

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
#include "verilatorSSTAPI.h"

namespace SST::VerilatorSST {

class VerilatorComponent : public SST::Component {
public:
  /// VerilatorComponent: constuctor
  VerilatorComponent(SST::ComponentId_t id, const SST::Params& params);

  /// VerilatorComponent: destructor
  ~VerilatorComponent();

  /// VerilatorComponent: setup function
  void setup();

  /// VerilatorComponent: finish function
  void finish();

  /// VerilatorComponent: init function
  void init( unsigned int phase );

  /// VerilatorComponent: clock function
  bool clock(SST::Cycle_t currentCycle );

  // -------------------------------------------------------
  // VerilatorComponent Component Registration Data
  // -------------------------------------------------------
  SST_ELI_REGISTER_COMPONENT(
    VerilatorComponent,  // component class
    "verilatorcomponent",// component library
    "VerilatorComponent",// component name
    SST_ELI_ELEMENT_VERSION( 1, 0, 0 ),
    "VerilatorSST Component Shell",
    COMPONENT_CATEGORY_UNCATEGORIZED
  )

  // -------------------------------------------------------
  // VerilatorComponent Component Parameter Data
  // -------------------------------------------------------
  // clang-format off
  SST_ELI_DOCUMENT_PARAMS(
    {"verbose",     "Sets the verbosity",       "0"},
    {"clockFreq",   "Clock frequency",          "1GHz"},
    {"numCycles",   "Number of cycles to exec", "1000"},
  )

  // -------------------------------------------------------
  // VerilatorComponent Port Parameter Data
  // -------------------------------------------------------
  SST_ELI_DOCUMENT_PORTS(
  )

  // -------------------------------------------------------
  // VerilatorComponent SubComponent Parameter Data
  // -------------------------------------------------------
  SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS(
    {"model", "Verilator Subcomponent Model",   "SST::VerilatorSST::VerilatorSSTBase"},
  )

private:
  SST::Output    output;                          ///< VerilatorComponent: SST output

  uint64_t NumCycles;                             ///< VerilatorComponent: number of cycles to execute

  SST::VerilatorSST::VerilatorSSTBase *model;     ///< VerilatorComponent: subcomponent model

};  // class VerilatorComponent

};  // namespace SST::VerilatorSST

#endif  // _VERILATOR_COMPONENT_H_

// EOF
