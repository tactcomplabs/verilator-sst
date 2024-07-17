#include "testHarness.h"

namespace SST::VerilatorSST{
class AccumTestBench : public TestHarness {
    void runTestSuite(SST::Cycle_t currentCycle) override;

public:
    AccumTestBench(SST::ComponentId_t id, const SST::Params& params)
        : TestHarness(id,params){};

    SST_ELI_REGISTER_COMPONENT(
        AccumTestBench,
        "verilatorssttests",
        "AccumTestBench",
        SST_ELI_ELEMENT_VERSION( 1, 0, 0 ),
        "VerilatorSST read port test bench component",
        COMPONENT_CATEGORY_UNCATEGORIZED
    )
};
}
