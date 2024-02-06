//
// _basicComponent_h_
//

#ifndef _BASIC_COMPONENT_H_
#define _BASIC_COMPONENT_H_

#include "Top.h"
#include <sst/core/component.h>

namespace SST {
namespace basicComponent {

class basicVerilogCounter : public SST::Component
{
public:

  // Register the component with the SST element library
  SST_ELI_REGISTER_COMPONENT(
    basicVerilogCounter,                               // Component class
    "basicComponent",                         // Component library
    "basicVerilogCounter",                             // Component name
    SST_ELI_ELEMENT_VERSION(1,0,0),           // Version of the component
    "basicVerilogCounter: simple clocked component",   // Description of the component
    COMPONENT_CATEGORY_UNCATEGORIZED          // Component category
  )

  // Document the parameters that this component accepts
  // { "parameter_name", "description", "default value or NULL if required" }
  SST_ELI_DOCUMENT_PARAMS(
    { "clockFreq",  "Frequency of perid (with units) of the clock", "1GHz" },
    { "maxCount", "Number of clock ticks to execute",             "10"  }
  )

  // [Optional] Document the ports: we do not define any 
  SST_ELI_DOCUMENT_PORTS()

  // [Optional] Document the statisitcs: we do not define any 
  SST_ELI_DOCUMENT_STATISTICS()

  // [Optional] Document the subcomponents: we do not define any 
  SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS()

  // Constructor: Components receive a unique ID and the set of parameters
  //              that were assigned in the simulation configuration script
  basicVerilogCounter(SST::ComponentId_t id, SST::Params& params);

  // Destructor
  ~basicVerilogCounter();

private:

  // Clock handler
  bool clock(SST::Cycle_t cycle);

  // Params
  SST::Output* out;       // SST Output object for printing, messaging, etc
  std::string clockFreq;  // Clock frequency
  uint64_t max;     // Cycle counter

  Top* top;
  VerilatedContext* contextp;
  void verilatorSetup();

};  // class basicClocks
}   // namespace basicComponent
}   // namespace SST

#endif
