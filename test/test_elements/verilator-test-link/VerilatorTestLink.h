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

struct PortDef {
  uint32_t PortId;
  uint32_t Size;
  bool Write; // true for writing (input) ports, false for reading (output) ports

  // Default constructor
  PortDef() : PortId( 0 ), Size( 0 ), Write( false ) { }

  // Full constructor
  PortDef( uint32_t PortId, uint32_t Size, bool Write ) :
              PortId( PortId ), Size( Size ), Write( Write ) { }
};

struct TestOp {
  uint32_t PortId;
  uint64_t * Values;
  uint64_t AtTick;

  // Default constructor
  TestOp() : PortId( 0 ), Values( nullptr ), AtTick( 0 ) { }

  // Full constructor
  TestOp( uint32_t PortId, uint64_t * Values, uint64_t AtTick ) : 
          PortId( PortId ), Values( Values ), AtTick( AtTick ) { }
};

class VerilatorTestLink : public SST::Component {
public:
  /// VerilatorTestLink: constructor
  VerilatorTestLink(SST::ComponentId_t id, const SST::Params& params);

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

  /// VerilatorTestLink: Splits a parameter array into tokens of std::string values
  void splitStr(const std::string& s, char c, std::vector<std::string>& v);

  /// VerilatorTestLink: helper function to break a larger uint into bytes and add to a uint8 vector
  template<typename T>
  void AddToPacket( T Data, std::vector<uint8_t>& Packet ) {
    for( size_t i = 0; i < sizeof( T ); i++ ) {
      Packet.push_back( ( Data >> ( i * 8 ) ) & 255 );
    }
  }

  const TestOp ConvertToTestOp( const std::string& StrOp ) {
    std::vector<std::string> op;
    splitStr( StrOp, ':', op );
    const PortDef portInfo = PortMap[op[0]];
    const uint32_t id = portInfo.PortId;
    uint32_t size = portInfo.Size;
    uint32_t nvals = size / 8;
    const  uint32_t rem = (size % 8 == 0) ? 0 : 1;
    uint64_t * const values = new uint64_t[nvals+rem];
    for (size_t i=0; i<nvals; i++) {
      const uint64_t val = std::stoull( op[1+i] );
      values[i] = val;
      size -= 8;
    }
    if ( rem ) {
      const uint64_t val = std::stoull( op[1+nvals] );
      values[nvals] = val;
      nvals++;
    }
    const uint64_t tick = std::stoull( op[1+nvals] );
    const TestOp toRet( id, values, tick );
    return toRet;
  }

  // -------------------------------------------------------
  // VerilatorTestLink Component Registration Data
  // -------------------------------------------------------
  SST_ELI_REGISTER_COMPONENT(
    VerilatorTestLink,    // component class
    "verilatortestlink",  // component library
    "VerilatorTestLink",  // component name
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
    {"num_ports",   "Number of ports",          "0"},
    {"portMap",     "portname:id:size:direction pairings",     "" },
    {"testFile",    "name of file holding test ops", ""},
    {"testOps",     "List of 'portname:vals:tick' strings to drive testing", ""},
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
  std::map<std::string, PortDef> PortMap;
  std::vector<PortDef> InfoVec;
  SST::Link ** Links;
  std::queue<TestOp> OpQueue;
  std::queue<std::vector<uint8_t>> ReadDataCheck;
  uint64_t currTick = 0;

  void InitPortMap( const SST::Params& params );
  void InitLinkConfig( const SST::Params& params );
  void InitTestOps( const SST::Params& params );
  void RecvPortEvent( SST::Event* ev, unsigned portId );
  bool ExecTestOp();

};  // class VerilatorTestLink
};  // namespace SST::VerilatorSST

#endif  // _VERILATOR_TEST_LINK_H_
