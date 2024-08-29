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

struct TestOp {
  std::string PortName;
  uint64_t * Values;
  uint64_t AtTick;
  bool isWrite; //TODO same code in VerilatorTestLInk.h

  // Default constructor
  TestOp() : PortName( 0 ), isWrite(false), Values( nullptr ), AtTick( 0 ) { }

  // Full constructor
  TestOp( const std::string PortName, bool isWrite, uint64_t * Values, uint64_t AtTick ) : 
          PortName( PortName ), isWrite(isWrite), Values( Values ), AtTick( AtTick ) { }
};

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

  /// VerilatorTestDirect: Splits a parameter array into tokens of std::string values
  void splitStr(const std::string& s, char c, std::vector<std::string>& v);

  /// VerilatorTestDirect: helper function to break a larger uint into bytes and add to a uint8 vector
  template<typename T>
  void AddToPacket( T Data, std::vector<uint8_t>& Packet ) {
    for( size_t i = 0; i < sizeof( T ); i++ ) {
      Packet.push_back( ( Data >> ( i * 8 ) ) & 255 );
    }
  }

  const TestOp ConvertToTestOp( const std::string& StrOp ) {
    std::vector<std::string> op;
    splitStr( StrOp, ':', op );
    uint32_t width;
    uint32_t depth;
    const bool isWrite = strcmp(op[1].c_str(), "write") == 0;
    model->getPortWidth(op[0], width); // in bits
    const uint32_t byteWidth = ( width / 8 ) + ( ( width % 8 == 0 ) ? 0 : 1 );
    model->getPortDepth(op[0], depth);
    uint32_t size = byteWidth * depth;
    uint32_t nvals = size / 8;
    const  uint32_t rem = (size % 8 == 0) ? 0 : 1;
    uint64_t * const values = new uint64_t[nvals+rem];
    for (size_t i=0; i<nvals; i++) {
      const uint64_t val = std::stoull( op[2+i] );
      values[i] = val;
      size -= 8;
    }
    if ( rem ) {
      const uint64_t val = std::stoull( op[2+nvals] );
      values[nvals] = val;
      nvals++;
    }
    const uint64_t tick = std::stoull( op[2+nvals] );
    const TestOp toRet( op[0], isWrite, values, tick );
    return toRet;
  }

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
    {"clockFreq",   "Clock frequency",          "1GHz"},
    {"testFile",    "name of file holding test ops", ""},
    {"testOps",     "List of 'portname:vals:tick' strings to drive testing", ""},
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
  std::queue<TestOp> OpQueue;
  uint64_t currTick = 0;

  void InitTestOps( const SST::Params& params );
  bool ExecTestOp();
 
  std::vector<uint8_t> generateData(unsigned Width, unsigned Depth);  ///< VerilatorTestDirect: generate random input data

};  // class VerilatorTestDirect

};  // namespace SST::VerilatorSST

#endif  // _VERILATOR_TEST_DIRECT_H_

// EOF
