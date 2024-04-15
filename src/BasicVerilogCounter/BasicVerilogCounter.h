//
// _BasicVerilogCounter_h_
//

#ifndef _BASIC_VERILOG_COUNTER_H_
#define _BASIC_VERILOG_COUNTER_H_

#include <memory>
#include <cassert>
#include <sst/core/sst_config.h>
#include <sst/core/component.h>
#include "verilatorSST.h"
#include "VCounter.h"

namespace SST::VerilatorSST {

class BasicVerilogCounter : public SST::Component
{
public:

  // Register the component with the SST element library
  SST_ELI_REGISTER_COMPONENT(
    BasicVerilogCounter,                               // Component class
    "verilatorsst",                         // Component library
    "BasicVerilogCounter",                             // Component name
    SST_ELI_ELEMENT_VERSION(1,0,0),           // Version of the component
    "BasicVerilogCounter: simple clocked component",   // Description of the component
    COMPONENT_CATEGORY_UNCATEGORIZED          // Component category
  )

  // Document the parameters that this component accepts
  // { "parameter_name", "description", "default value or NULL if required" }
  SST_ELI_DOCUMENT_PARAMS(
    { "CLOCK_FREQ",  "Frequency of perid (with units) of the clock", "1GHz" },
    { "STOP", "Number of clock ticks to execute",             "20"  },
    { "STOP_WIDTH", "Width of stop signal",             "8"  }
  )

  // [Optional] Document the ports: we do not define any 
  SST_ELI_DOCUMENT_PORTS()

  // [Optional] Document the statisitcs: we do not define any 
  SST_ELI_DOCUMENT_STATISTICS()

  // [Optional] Document the subcomponents: we do not define any 
  SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS()

  // Constructor: Components receive a unique ID and the set of parameters
  //              that were assigned in the simulation configuration script
  BasicVerilogCounter(SST::ComponentId_t id, SST::Params& params);

  // Destructor
  ~BasicVerilogCounter();

private:
  SST::Output* out;       // SST Output object for printing, messaging, etc
  std::unique_ptr<VerilatorSST<VCounter>> top;
  uint32_t stopWidth;
  
  // Clock handler
  bool clock(SST::Cycle_t cycle);
  bool testBenchPass();
  void verilatorSetup(uint16_t stop);
};  // class basicClocks
}   // namespace SST::VerilatorSST

#endif
