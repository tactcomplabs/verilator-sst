#include "TestHarness.h"

namespace SST::VerilatorSST{
class $TEST_BENCH_CLASS : public TestHarness {
    void runTestSuite(SST::Cycle_t currentCycle) override;

public:
    $TEST_BENCH_CLASS(SST::ComponentId_t id, const SST::Params& params)
        : TestHarness(id,params){};

    SST_ELI_REGISTER_COMPONENT(
        $TEST_BENCH_CLASS,
        "verilatorssttests",
        "$TEST_BENCH_CLASS",
        SST_ELI_ELEMENT_VERSION( 1, 0, 0 ),
        "VerilatorSST read port test bench component",
        COMPONENT_CATEGORY_UNCATEGORIZED
    )
};
}
