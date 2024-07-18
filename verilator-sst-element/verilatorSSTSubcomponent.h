//
// _verilatorSSTSubcomponent_h_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _DUT_SUBCOMPONENT_H_
#define _DUT_SUBCOMPONENT_H_

// -- Standard Headers
#include <vector>
#include <string>
#include <tuple>
#include <list>
#include <cassert>

// -- SST Headers
#include "SST.h"

// -- Component Headers
//#include "Signal.h"

// -- Verilator Headers
#include "VTop.h"
#include "verilated.h"
#include "verilated_vpi.h"

namespace SST::VerilatorSST {

enum class VPortType : uint8_t {
  V_INPUT   = 0b00000000,       /// VPortType: input port
  V_OUTPUT  = 0b00000001,       /// VPortType: output port
};

// ---------------------------------------------------------------
// Macros
// ---------------------------------------------------------------
#ifdef _VERILATORSST_DEBUG_
#define VL_DEBUG 1
#else
#define VL_DEBUG 0
#endif

// ---------------------------------------------------------------
// Data Structures
// ---------------------------------------------------------------
typedef void (*DirectWriteFunc)(VTop*, const std::vector<uint8_t>&);
typedef std::vector<uint8_t> (*DirectReadFunc)(VTop*);

#define V_NAME            0
#define V_TYPE            1
#define V_WIDTH           2
#define V_DEPTH           3
#define V_WRITEFUNC       4
#define V_READFUNC        5
typedef std::tuple<std::string,
                   VPortType,
                   unsigned, //width
                   unsigned, //depth
                   DirectWriteFunc,
                   DirectReadFunc> PortEntry;

typedef std::pair<std::string,
                  uint64_t> PortReset;

struct QueueEntry {
  std::string PortName;
  uint64_t AtTick;
  std::vector<uint8_t> Packet;
  QueueEntry(std::string PortName, uint64_t AtTick, std::vector<uint8_t> Packet)
      : PortName(PortName), AtTick(AtTick), Packet(Packet) { }
};


// ---------------------------------------------------------------
// PortEvent
// ---------------------------------------------------------------
class PortEvent : public SST::Event{
public:
  /// PortEvent: default constructor
  explicit PortEvent()
    : Event(), AtTick(0x00ull) {
  }

  explicit PortEvent(uint64_t Tick)
    : Event(), AtTick(Tick) {
  }

  explicit PortEvent(std::vector<uint8_t> P)
    : Event(), AtTick(0x00ull) {
    std::copy(P.begin(), P.end(),
              std::back_inserter(Packet));
  }

  explicit PortEvent(std::vector<uint8_t> P, uint64_t Tick)
    : Event(), AtTick(Tick) {
    std::copy(P.begin(), P.end(),
              std::back_inserter(Packet));
  }

  /// PortEvent: virtual clone function
  virtual Event* clone(void) override{
    PortEvent *pe = new PortEvent(*this);
    return pe;
  }

  /// PortEvent: retrieve the target clock tick
  uint64_t getAtTick() { return AtTick; }

  /// PortEvent: retrieve the packet payload
  const std::vector<uint8_t> getPacket() { return Packet; }

  /// PortEvent: set the target clock tick
  void setAtTick(uint64_t T) { AtTick = T; }

  /// PortEvent: set the payload
  void setPayload(const std::vector<uint8_t> P){
    std::copy(P.begin(), P.end(),
              std::back_inserter(Packet));
  }

private:
  std::vector<uint8_t> Packet;  /// event packet
  uint64_t AtTick;              /// event at clock tick

public:
  // PortEvent: event serializer
  void serialize_order(SST::Core::Serialization::serializer &ser) override{
    // we only serialize the raw packet
    Event::serialize_order(ser);
    ser & Packet;
    ser & AtTick;
  }

  // PortEvent: implements the nic serialization
  ImplementSerializable(SST::VerilatorSST::PortEvent);
};

// ---------------------------------------------------------------
// SignalHelper
// ---------------------------------------------------------------
class SignalHelper{
private:
  unsigned Width; ///< signal width

public:

  /// SignalHelper Constructor
  SignalHelper() : Width(1){};

  /// SignalHelper Overloaded Constructor
  SignalHelper(unsigned W) : Width(W){};

  /// SignalHelper destructor
  ~SignalHelper() {}

  /// retrieve the width
  unsigned getWidth() { return Width; }

  /// set the width
  void setWidth(unsigned W) { Width = W; }

  /// template class to generate scalar masks
  template<typename T>
  T getMask(){
    T ret = 0x00;
    for( unsigned i=0; i<Width; i++ ){
      ret |= (1<<i);
    }
    return ret;
  }

}; // SignalHelper

// ---------------------------------------------------------------
// VerilatorSSTBase
// ---------------------------------------------------------------
class VerilatorSSTBase : public SST::SubComponent{
public:
  SST_ELI_REGISTER_SUBCOMPONENT_API(SST::VerilatorSST::VerilatorSSTBase)

  SST_ELI_DOCUMENT_PARAMS(
    { "verbose",      "Set the verbosity of output for the device", "0" },
  )

  /// VerilatorSSTBase: constructor
  VerilatorSSTBase( std::string DerivedName,
                    ComponentId_t id, const Params& params );

  /// VerilatorSSTBase: virtual destructor
  virtual ~VerilatorSSTBase();

  /// VerilatorSSTBase: initialization function
  virtual void init(unsigned int phase) = 0;

  /// VerilatorSSTBase: setup function
  virtual void setup() = 0;

  /// VerilatorSSTBase: finish function
  virtual void finish() = 0;

  /// VerliatorSSTBase: clock tick function
  virtual bool clock(SST::Cycle_t cycle) = 0;

  /// VerilatorSSTBase: get the current clock tick from verilator
  virtual uint64_t getCurrentTick() = 0;

  /// VerilatorSSTBase: determine if the target port is valid
  virtual bool isNamedPort(std::string PortName) = 0;

  /// VerilatorSSTBase: retrieve the number of configured ports
  virtual unsigned getNumPorts() = 0;

  /// VerilatorSSTBase: retrieve a vector of all the port names
  virtual const std::vector<std::string> getPortsNames() = 0;

  /// VerilatorSSTBase: retrieve the port type of the target port
  virtual bool getPortType(std::string PortName, VPortType& Type) = 0;

  /// VerilatorSSTBase: retrieve the port width of the target port
  virtual bool getPortWidth(std::string PortName, unsigned& Width) = 0;

  /// VerilatorSSTBase: retrieve the port depth of the target port
  virtual bool getPortDepth(std::string PortName, unsigned& Depth) = 0;

  /// VerilatorSSTBase: retrieve the port reset value of the target port
  virtual bool getResetVal(std::string PortName, uint64_t& Val) = 0;

  /// VerilatorSSTBase: write to the target port
  virtual void writePort(std::string portName,
                         const std::vector<uint8_t>& packet) = 0;

  /// VerilatorSSTBase: write to the target port at the target clock cycle
  virtual void writePortAtTick(std::string portName,
                               const std::vector<uint8_t>& packet,
                               uint64_t tick) = 0;

  /// VerilatorSSTBase: read from the target port
  virtual std::vector<uint8_t> readPort(std::string portName) = 0;

protected:
  SST::Output *output;        ///< VerilatorSST: SST output handler
  uint32_t verbosity;         ///< VerilatorSST: verbosity parameter

};  // class VerilatorSST

// ---------------------------------------------------------------
// VerilatorSSTDUT
// ---------------------------------------------------------------
class VerilatorSSTDUT : public VerilatorSSTBase{
public:
  SST_ELI_REGISTER_SUBCOMPONENT(VerilatorSSTDUT,
                                "verilatorsstDUT",
                                "VerilatorSSTDUT",
                                SST_ELI_ELEMENT_VERSION(1, 0, 0),
                                "Verilator SST DUT Wrapper",
                                SST::VerilatorSST::VerilatorSSTBase
  )

  // Set up parameters accesible from the python configuration
  SST_ELI_DOCUMENT_PARAMS(
    { "useVPI",     "Is Verilator VPI used",                      "false"},
    { "clockFreq",  "Sets the clock frequency",                   "1GHz"},
    { "clockPort",  "Sets the internal verilog clock port",       "clock"},
    { "resetVals",  "Initial reset values for each labeled port", "port:Val"},
  )

  // Register any subcomponents used by this element
  SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS()

  // Register any ports used with this element
  SST_ELI_DOCUMENT_PORTS(
  {"write_port", "Input Port", {"SST::VerilatorSST::PortEvent"} },
{"write_byte_port", "Input Port", {"SST::VerilatorSST::PortEvent"} },
{"write_reg_port", "Input Port", {"SST::VerilatorSST::PortEvent"} },
{"write_wire_port", "Input Port", {"SST::VerilatorSST::PortEvent"} },
{"write_half_port", "Input Port", {"SST::VerilatorSST::PortEvent"} },
{"write_word_port", "Input Port", {"SST::VerilatorSST::PortEvent"} },
{"write_quad_port", "Input Port", {"SST::VerilatorSST::PortEvent"} },
{"write_double_port", "Input Port", {"SST::VerilatorSST::PortEvent"} },
{"write_array_port", "Input Port", {"SST::VerilatorSST::PortEvent"} },
{"write_quad_array_port", "Input Port", {"SST::VerilatorSST::PortEvent"} },
{"read_write_port", "Output port", {"SST::VerilatorSST::PortEvent"} },
{"read_write_byte_port", "Output port", {"SST::VerilatorSST::PortEvent"} },
{"read_write_reg_port", "Output port", {"SST::VerilatorSST::PortEvent"} },
{"read_write_wire_port", "Output port", {"SST::VerilatorSST::PortEvent"} },
{"read_write_half_port", "Output port", {"SST::VerilatorSST::PortEvent"} },
{"read_write_word_port", "Output port", {"SST::VerilatorSST::PortEvent"} },
{"read_write_quad_port", "Output port", {"SST::VerilatorSST::PortEvent"} },
{"read_write_double_port", "Output port", {"SST::VerilatorSST::PortEvent"} },
{"read_write_array_port", "Output port", {"SST::VerilatorSST::PortEvent"} },
{"read_write_quad_array_port", "Output port", {"SST::VerilatorSST::PortEvent"} },
  )

  // Add statistics
  SST_ELI_DOCUMENT_STATISTICS()

  /// default constructor
  VerilatorSSTDUT(ComponentId_t id, const Params& params);

  /// default destructor
  virtual ~VerilatorSSTDUT();

  /// initialization function
  virtual void init(unsigned int phase) override;

  /// setup function
  virtual void setup() override;

  /// finish function
  virtual void finish() override;

  /// clock tick function
  virtual bool clock(SST::Cycle_t cycle) override;

  /// get the current clock tick from verilator
  virtual uint64_t getCurrentTick() override;

  /// determine if the target port is valid
  virtual bool isNamedPort(std::string PortName) override;

  /// retrieve the number of configured ports
  virtual unsigned getNumPorts() override;

  /// retrieve a vector of all the port names
  virtual const std::vector<std::string> getPortsNames() override;

  /// retrieve the port type of the target port
  virtual bool getPortType(std::string PortName,
                           SST::VerilatorSST::VPortType& Type) override;

  /// retrieve the port width of the target port
  virtual bool getPortWidth(std::string PortName, unsigned& Width) override;

  /// retrieve the port depth of the target port
  virtual bool getPortDepth(std::string PortName, unsigned& Depth) override;

  /// retrieve the port reset value of the target port
  virtual bool getResetVal(std::string PortName, uint64_t& Val) override;

  /// write to the target port
  virtual void writePort(std::string portName,
                         const std::vector<uint8_t>& packet) override;

  /// write to the target port at the target clock cycle
  virtual void writePortAtTick(std::string portName,
                               const std::vector<uint8_t>& packet,
                               uint64_t tick) override;

  /// read from the target port
  virtual std::vector<uint8_t> readPort(std::string portName) override;

private:

  // Private data
  bool UseVPI;          ///< Is the verilator VPI interface used?
  VerilatedContext *ContextP;       ///< verilated context
  VTop *Top;                        ///< top module
  std::list<QueueEntry> WriteQueue; ///< port write queue
  // Links for each port
  

  // Private functions

  /// Check for write packets in the queue that need to be performed this tick
  void pollWriteQueue();

  /// Initializes the internal reset values for each port from the parameter list
  void initResetValues(const Params& params);

  /// Splits a parameter array into tokens of std::string values
  void splitStr(const std::string& s, char c, std::vector<std::string>& v);

  /// VPI Read of Port
  std::vector<uint8_t> readPortVPI(std::string PortName);

  /// VPI Write of Port
  void writePortVPI(std::string PortName,
                    const std::vector<uint8_t>& Packet);

  static void DirectWritewrite_port(VTop *, const std::vector<uint8_t>&);
static std::vector<uint8_t> DirectReadwrite_port(VTop *);
static void DirectWritewrite_byte_port(VTop *, const std::vector<uint8_t>&);
static std::vector<uint8_t> DirectReadwrite_byte_port(VTop *);
static void DirectWritewrite_reg_port(VTop *, const std::vector<uint8_t>&);
static std::vector<uint8_t> DirectReadwrite_reg_port(VTop *);
static void DirectWritewrite_wire_port(VTop *, const std::vector<uint8_t>&);
static std::vector<uint8_t> DirectReadwrite_wire_port(VTop *);
static void DirectWritewrite_half_port(VTop *, const std::vector<uint8_t>&);
static std::vector<uint8_t> DirectReadwrite_half_port(VTop *);
static void DirectWritewrite_word_port(VTop *, const std::vector<uint8_t>&);
static std::vector<uint8_t> DirectReadwrite_word_port(VTop *);
static void DirectWritewrite_quad_port(VTop *, const std::vector<uint8_t>&);
static std::vector<uint8_t> DirectReadwrite_quad_port(VTop *);
static void DirectWritewrite_double_port(VTop *, const std::vector<uint8_t>&);
static std::vector<uint8_t> DirectReadwrite_double_port(VTop *);
static void DirectWritewrite_array_port(VTop *, const std::vector<uint8_t>&);
static std::vector<uint8_t> DirectReadwrite_array_port(VTop *);
static void DirectWritewrite_quad_array_port(VTop *, const std::vector<uint8_t>&);
static std::vector<uint8_t> DirectReadwrite_quad_array_port(VTop *);
static void DirectWriteread_write_port(VTop *, const std::vector<uint8_t>&);
static std::vector<uint8_t> DirectReadread_write_port(VTop *);
static void DirectWriteread_write_byte_port(VTop *, const std::vector<uint8_t>&);
static std::vector<uint8_t> DirectReadread_write_byte_port(VTop *);
static void DirectWriteread_write_reg_port(VTop *, const std::vector<uint8_t>&);
static std::vector<uint8_t> DirectReadread_write_reg_port(VTop *);
static void DirectWriteread_write_wire_port(VTop *, const std::vector<uint8_t>&);
static std::vector<uint8_t> DirectReadread_write_wire_port(VTop *);
static void DirectWriteread_write_half_port(VTop *, const std::vector<uint8_t>&);
static std::vector<uint8_t> DirectReadread_write_half_port(VTop *);
static void DirectWriteread_write_word_port(VTop *, const std::vector<uint8_t>&);
static std::vector<uint8_t> DirectReadread_write_word_port(VTop *);
static void DirectWriteread_write_quad_port(VTop *, const std::vector<uint8_t>&);
static std::vector<uint8_t> DirectReadread_write_quad_port(VTop *);
static void DirectWriteread_write_double_port(VTop *, const std::vector<uint8_t>&);
static std::vector<uint8_t> DirectReadread_write_double_port(VTop *);
static void DirectWriteread_write_array_port(VTop *, const std::vector<uint8_t>&);
static std::vector<uint8_t> DirectReadread_write_array_port(VTop *);
static void DirectWriteread_write_quad_array_port(VTop *, const std::vector<uint8_t>&);
static std::vector<uint8_t> DirectReadread_write_quad_array_port(VTop *);

  void handle_write_port(SST::Event* ev);
void handle_write_byte_port(SST::Event* ev);
void handle_write_reg_port(SST::Event* ev);
void handle_write_wire_port(SST::Event* ev);
void handle_write_half_port(SST::Event* ev);
void handle_write_word_port(SST::Event* ev);
void handle_write_quad_port(SST::Event* ev);
void handle_write_double_port(SST::Event* ev);
void handle_write_array_port(SST::Event* ev);
void handle_write_quad_array_port(SST::Event* ev);
void handle_read_write_port(SST::Event* ev);
void handle_read_write_byte_port(SST::Event* ev);
void handle_read_write_reg_port(SST::Event* ev);
void handle_read_write_wire_port(SST::Event* ev);
void handle_read_write_half_port(SST::Event* ev);
void handle_read_write_word_port(SST::Event* ev);
void handle_read_write_quad_port(SST::Event* ev);
void handle_read_write_double_port(SST::Event* ev);
void handle_read_write_array_port(SST::Event* ev);
void handle_read_write_quad_array_port(SST::Event* ev);

  // Private data
  std::string clockPort;                  ///< verilator named clock port

  ///< Map of port indices to reset values
  std::vector<PortReset> ResetVals;

  ///< Vector of port descriptor tuples
  const std::vector<PortEntry>  Ports = {
    {"write_port", SST::VerilatorSST::VPortType::V_INPUT, 1, 1, SST::VerilatorSST::VerilatorSSTDUT::DirectWritewrite_port, SST::VerilatorSST::VerilatorSSTDUT::DirectReadwrite_port },
{"write_byte_port", SST::VerilatorSST::VPortType::V_INPUT, 8, 1, SST::VerilatorSST::VerilatorSSTDUT::DirectWritewrite_byte_port, SST::VerilatorSST::VerilatorSSTDUT::DirectReadwrite_byte_port },
{"write_reg_port", SST::VerilatorSST::VPortType::V_INPUT, 1, 1, SST::VerilatorSST::VerilatorSSTDUT::DirectWritewrite_reg_port, SST::VerilatorSST::VerilatorSSTDUT::DirectReadwrite_reg_port },
{"write_wire_port", SST::VerilatorSST::VPortType::V_INPUT, 1, 1, SST::VerilatorSST::VerilatorSSTDUT::DirectWritewrite_wire_port, SST::VerilatorSST::VerilatorSSTDUT::DirectReadwrite_wire_port },
{"write_half_port", SST::VerilatorSST::VPortType::V_INPUT, 16, 1, SST::VerilatorSST::VerilatorSSTDUT::DirectWritewrite_half_port, SST::VerilatorSST::VerilatorSSTDUT::DirectReadwrite_half_port },
{"write_word_port", SST::VerilatorSST::VPortType::V_INPUT, 32, 1, SST::VerilatorSST::VerilatorSSTDUT::DirectWritewrite_word_port, SST::VerilatorSST::VerilatorSSTDUT::DirectReadwrite_word_port },
{"write_quad_port", SST::VerilatorSST::VPortType::V_INPUT, 128, 1, SST::VerilatorSST::VerilatorSSTDUT::DirectWritewrite_quad_port, SST::VerilatorSST::VerilatorSSTDUT::DirectReadwrite_quad_port },
{"write_double_port", SST::VerilatorSST::VPortType::V_INPUT, 64, 1, SST::VerilatorSST::VerilatorSSTDUT::DirectWritewrite_double_port, SST::VerilatorSST::VerilatorSSTDUT::DirectReadwrite_double_port },
{"write_array_port", SST::VerilatorSST::VPortType::V_INPUT, 1, 128, SST::VerilatorSST::VerilatorSSTDUT::DirectWritewrite_array_port, SST::VerilatorSST::VerilatorSSTDUT::DirectReadwrite_array_port },
{"write_quad_array_port", SST::VerilatorSST::VPortType::V_INPUT, 128, 128, SST::VerilatorSST::VerilatorSSTDUT::DirectWritewrite_quad_array_port, SST::VerilatorSST::VerilatorSSTDUT::DirectReadwrite_quad_array_port },
{"read_write_port", SST::VerilatorSST::VPortType::V_OUTPUT, 1, 1, SST::VerilatorSST::VerilatorSSTDUT::DirectWriteread_write_port, SST::VerilatorSST::VerilatorSSTDUT::DirectReadread_write_port },
{"read_write_byte_port", SST::VerilatorSST::VPortType::V_OUTPUT, 8, 1, SST::VerilatorSST::VerilatorSSTDUT::DirectWriteread_write_byte_port, SST::VerilatorSST::VerilatorSSTDUT::DirectReadread_write_byte_port },
{"read_write_reg_port", SST::VerilatorSST::VPortType::V_OUTPUT, 1, 1, SST::VerilatorSST::VerilatorSSTDUT::DirectWriteread_write_reg_port, SST::VerilatorSST::VerilatorSSTDUT::DirectReadread_write_reg_port },
{"read_write_wire_port", SST::VerilatorSST::VPortType::V_OUTPUT, 1, 1, SST::VerilatorSST::VerilatorSSTDUT::DirectWriteread_write_wire_port, SST::VerilatorSST::VerilatorSSTDUT::DirectReadread_write_wire_port },
{"read_write_half_port", SST::VerilatorSST::VPortType::V_OUTPUT, 16, 1, SST::VerilatorSST::VerilatorSSTDUT::DirectWriteread_write_half_port, SST::VerilatorSST::VerilatorSSTDUT::DirectReadread_write_half_port },
{"read_write_word_port", SST::VerilatorSST::VPortType::V_OUTPUT, 32, 1, SST::VerilatorSST::VerilatorSSTDUT::DirectWriteread_write_word_port, SST::VerilatorSST::VerilatorSSTDUT::DirectReadread_write_word_port },
{"read_write_quad_port", SST::VerilatorSST::VPortType::V_OUTPUT, 128, 1, SST::VerilatorSST::VerilatorSSTDUT::DirectWriteread_write_quad_port, SST::VerilatorSST::VerilatorSSTDUT::DirectReadread_write_quad_port },
{"read_write_double_port", SST::VerilatorSST::VPortType::V_OUTPUT, 64, 1, SST::VerilatorSST::VerilatorSSTDUT::DirectWriteread_write_double_port, SST::VerilatorSST::VerilatorSSTDUT::DirectReadread_write_double_port },
{"read_write_array_port", SST::VerilatorSST::VPortType::V_OUTPUT, 1, 128, SST::VerilatorSST::VerilatorSSTDUT::DirectWriteread_write_array_port, SST::VerilatorSST::VerilatorSSTDUT::DirectReadread_write_array_port },
{"read_write_quad_array_port", SST::VerilatorSST::VPortType::V_OUTPUT, 128, 128, SST::VerilatorSST::VerilatorSSTDUT::DirectWriteread_write_quad_array_port, SST::VerilatorSST::VerilatorSSTDUT::DirectReadread_write_quad_array_port },
  };

  ///< Map of port names to descriptors
  const std::map<std::string, unsigned> PortMap = {
    {"write_port", 0 },
{"write_byte_port", 1 },
{"write_reg_port", 2 },
{"write_wire_port", 3 },
{"write_half_port", 4 },
{"write_word_port", 5 },
{"write_quad_port", 6 },
{"write_double_port", 7 },
{"write_array_port", 8 },
{"write_quad_array_port", 9 },
{"read_write_port", 10 },
{"read_write_byte_port", 11 },
{"read_write_reg_port", 12 },
{"read_write_wire_port", 13 },
{"read_write_half_port", 14 },
{"read_write_word_port", 15 },
{"read_write_quad_port", 16 },
{"read_write_double_port", 17 },
{"read_write_array_port", 18 },
{"read_write_quad_array_port", 19 },
  };
};

} // namespace SST::VerilatorSST

#endif
