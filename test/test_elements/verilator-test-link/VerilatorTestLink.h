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

// struct used to define characteristics of each exposed port
struct PortDef {
  uint32_t PortId;
  uint32_t Size;
  bool Write;
  bool Read;

  // Default constructor
  PortDef() : PortId( 0 ), Size( 0 ), Write( false ), Read ( false ) { }

  // Full constructor
  PortDef( uint32_t PortId, uint32_t Size, bool Write, bool Read ) :
              PortId( PortId ), Size( Size ), Write( Write ), Read( Read ) { }
};

// struct used to define each port operation used for testing
struct TestOp {
  uint32_t PortId;
  uint64_t * Values;
  uint64_t AtTick;
  bool isWrite;

  // Default constructor
  TestOp() : PortId( 0 ), isWrite(false), Values( nullptr ), AtTick( 0 ) { }

  // Full constructor
  TestOp( uint32_t PortId, bool isWrite, uint64_t * Values, uint64_t AtTick ) : 
          PortId( PortId ), isWrite(isWrite), Values( Values ), AtTick( AtTick ) { }
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

  /// VerilatorTestLink: function converting string from param to TestOp struct for use
  const TestOp ConvertToTestOp( const std::string& StrOp ) {
    std::vector<std::string> op;
    splitStr( StrOp, ':', op );
    const std::string portName = op[0];
    const bool isWrite = (op[1] == "write");
    const PortDef portInfo = PortMap[op[0]];
    const uint32_t id = portInfo.PortId;
    uint32_t size = portInfo.Size;
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
    const TestOp toRet( id, isWrite, values, tick );
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
  std::map<std::string, PortDef> PortMap;       ///< VerilatorTestLink: access port characteristics based on name
  std::vector<PortDef> InfoVec;                 ///< VerilatorTestLink: access port characteristics by ID number
  SST::Link ** Links;                           ///< VerilatorTestLink: list of links (one for each port in the verilator model)
  std::queue<TestOp> OpQueue;                   ///< VerilatorTestLink: queue holding test operations to be applied
  std::queue<std::vector<uint8_t>> ReadDataCheck; ///< VerilatorTestLink: queue to hold expected read data to check for correct read results
  uint64_t currTick = 0;                          ///< VerilatorTestLink: current tick of this test component

  void InitPortMap( const SST::Params& params );    ///< VerilatorTestLink: initialize name:port_info mapping
  void InitLinkConfig( const SST::Params& params ); ///< VerilatorTestLink: configure the links for each port
  void InitTestOps( const SST::Params& params );    ///< VerilatorTestLink: load in the test operations from params
  void RecvPortEvent( SST::Event* ev, unsigned portId );  ///< VerilatorTestLink: general port handler
  bool ExecTestOp();  ///< VerilatorTestLink: perform the next queued test operation

};  // class VerilatorTestLink
};  // namespace SST::VerilatorSST

#endif  // _VERILATOR_TEST_LINK_H_
