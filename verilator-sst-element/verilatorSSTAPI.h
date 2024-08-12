#include "SST.h"
namespace SST::VerilatorSST {

// ---------------------------------------------------------------
// Macros
// ---------------------------------------------------------------
#ifdef _VERILATORSST_DEBUG_
#define VL_DEBUG 1
#else
#define VL_DEBUG 0
#endif

#ifndef ENABLE_INOUT_HANDLING
#define ENABLE_INOUT_HANDLING 0
#endif

// ---------------------------------------------------------------
// Data Structures
// ---------------------------------------------------------------

enum class VPortType : uint8_t {
  V_INOUT  = 0b00000011,  /// VPortType: inout port
  V_INPUT  = 0b00000010,  /// VPortType: input port
  V_OUTPUT = 0b00000001,  /// VPortType: output port
};

#define V_NAME            0
#define V_TYPE             1
#define V_WIDTH           2
#define V_DEPTH           3
#define V_WRITEFUNC       4
#define V_READFUNC        5

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

enum class PortEventAction : uint8_t {
  WRITE = 0b00000000,
  READ  = 0b00000001
};

class PortEvent : public SST::Event{
public:
  /// PortEvent: default constructor
  explicit PortEvent()
    : Event(), AtTick(0x00ull), action(PortEventAction::READ) {
  }

  explicit PortEvent(uint64_t Tick, PortEventAction action)
    : Event(), AtTick(Tick), action(action) {
  }

  explicit PortEvent(std::vector<uint8_t> P)
    : Event(), AtTick(0x00ull), action(PortEventAction::WRITE) {
    std::copy(P.begin(), P.end(),
              std::back_inserter(Packet));
  }

  explicit PortEvent(std::vector<uint8_t> P, uint64_t Tick)
    : Event(), AtTick(Tick), action(PortEventAction::WRITE) {
    std::copy(P.begin(), P.end(),
              std::back_inserter(Packet));
  }

  /// PortEvent: virtual clone function
  virtual Event* clone(void) override{
    PortEvent *pe = new PortEvent(*this);
    return pe;
  }

  /// PortEvent: retrieve the packet action
  PortEventAction getAction() const { return action; }

  /// PortEvent: retrieve the target clock tick
  uint64_t getAtTick() const { return AtTick; }

  /// PortEvent: retrieve the packet payload
  const std::vector<uint8_t> getPacket() const { return Packet; }

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
  PortEventAction action;            /// event action

public:
  // PortEvent: event serializer
  void serialize_order(SST::Core::Serialization::serializer &ser) override{
    // we only serialize the raw packet
    Event::serialize_order(ser);
    ser & Packet;
    ser & AtTick;
    ser & action;
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
  virtual bool getPortType(std::string PortName, VPortType& direction) = 0;

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

}  // namespace SST::VerilatorSST
