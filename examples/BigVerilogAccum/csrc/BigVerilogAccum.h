//
// _BigVerilogAccum_h_
//

#ifndef _BIG_VERILOG_ACCUM_H_
#define _BIG_VERILOG_ACCUM_H_

#include <memory>
#include <cassert>
#include <sst/core/sst_config.h>
#include <sst/core/component.h>
#include "verilatorSST.h"
#include "VTop.h"

namespace SST::VerilatorSST {

class BigVerilogAccum : public SST::Component
{
public:

  // Register the component with the SST element library
  SST_ELI_REGISTER_COMPONENT(
    BigVerilogAccum,                               // Component class
    "verilatorsst",                         // Component library
    "BigVerilogAccum",                             // Component name
    SST_ELI_ELEMENT_VERSION(1,0,0),           // Version of the component
    "BigVerilogAccum: large sequential accumulation logic",   // Description of the component
    COMPONENT_CATEGORY_UNCATEGORIZED          // Component category
  )

  // Document the parameters that this component accepts
  // { "parameter_name", "description", "default value or NULL if required" }
  SST_ELI_DOCUMENT_PARAMS(
    { "CLOCK_FREQ",  "Frequency of period (with units) of the clock",   "1GHz" },
    { "ADD", "Initial value used to increment accumulator (lower 64)",  "20"  },
    { "ADD2", "Initial value used to increment accumulator (upper 64)", "20"  },
    { "ACCUM_WIDTH", "Width of accumulator/addition",                   "64"  }
  )

  // [Optional] Document the ports: we do not define any 
  SST_ELI_DOCUMENT_PORTS()

  // [Optional] Document the statisitcs: we do not define any 
  SST_ELI_DOCUMENT_STATISTICS()

  // [Optional] Document the subcomponents: we do not define any 
  SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS()

  // Constructor: Components receive a unique ID and the set of parameters
  //              that were assigned in the simulation configuration script
  BigVerilogAccum(SST::ComponentId_t id, SST::Params& params);

  // Destructor
  ~BigVerilogAccum();

private:
  SST::Output* out;       // SST Output object for printing, messaging, etc
  std::unique_ptr<VerilatorSST<VTop>> top;
  uint32_t accumWidth;
  int dawsonCounter;
  
  // Clock handler
  bool clock(SST::Cycle_t cycle);
  bool testBenchPass();
  void verilatorSetup(uint64_t * add);
};  // class basicClocks
}   // namespace SST::VerilatorSST

#endif
