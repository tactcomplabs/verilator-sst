#include "SST.h"
namespace SST::VerilatorSST {

enum class VPortType : uint8_t {
  V_INPUT = 0b00000000,  /// VPortType: input port
  V_OUTPUT = 0b00000001, /// VPortType: output port
};
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

}  // namespace SST::VerilatorSST
