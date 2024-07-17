#include "testHarness.h"

namespace SST::VerilatorSST{
class CounterTestBench : public TestHarness {
    void runTestSuite(SST::Cycle_t currentCycle) override;

public:
    CounterTestBench(SST::ComponentId_t id, const SST::Params& params)
        : TestHarness(id,params){};

    SST_ELI_REGISTER_COMPONENT(
        CounterTestBench,
        "verilatorssttests",
        "CounterTestBench",
        SST_ELI_ELEMENT_VERSION( 1, 0, 0 ),
        "VerilatorSST read port test bench component",
        COMPONENT_CATEGORY_UNCATEGORIZED
    )
};
}
