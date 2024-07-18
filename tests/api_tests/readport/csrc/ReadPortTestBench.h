#include "TestHarness.h"

namespace SST::VerilatorSST{
class ReadPortTestBench : public TestHarness {
    void runTestSuite(SST::Cycle_t currentCycle) override;

public:
    ReadPortTestBench(SST::ComponentId_t id, const SST::Params& params)
        : TestHarness(id,params){};

    SST_ELI_REGISTER_COMPONENT(
        ReadPortTestBench,  // component class
        "verilatorssttests",// component library
        "ReadPortTestBench",// component name
        SST_ELI_ELEMENT_VERSION( 1, 0, 0 ),
        "VerilatorSST read port test bench component",
        COMPONENT_CATEGORY_UNCATEGORIZED
    )
};
}
