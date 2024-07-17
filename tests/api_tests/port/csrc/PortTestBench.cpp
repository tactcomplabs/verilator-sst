#define N_PORTS 10

#include "PortTestBench.h"

using namespace SST::VerilatorSST;

void testIsNamedPort(SST::Output * output, SST::VerilatorSST::VerilatorSSTBase * dut){
    CHECK_TRUE(dut->isNamedPort("input_port"));
    CHECK_TRUE(dut->isNamedPort("output_port"));
    CHECK_TRUE(dut->isNamedPort("inout_port"));
    CHECK_TRUE(dut->isNamedPort("array_port"));
    CHECK_TRUE(dut->isNamedPort("wide_port"));
    CHECK_TRUE(dut->isNamedPort("wide_array_port"));
    CHECK_TRUE(dut->isNamedPort("logic_port"));
    CHECK_TRUE(dut->isNamedPort("reg_port"));
    CHECK_TRUE(dut->isNamedPort("wire_port"));
    CHECK_TRUE(dut->isNamedPort("_abcdefghijklmnopqrstuvwxyz_ABCDEFGHIJKLNMNOPQRSTUVWXYZ_1234567890_port"));
}

void testNumPorts(SST::Output * output, SST::VerilatorSST::VerilatorSSTBase * dut){
    CHECK_EQ(dut->getNumPorts(),N_PORTS);
}

void testGetPortNames(SST::Output * output, SST::VerilatorSST::VerilatorSSTBase * dut){
    const std::vector<std::string> & portNames = dut->getPortsNames();
    CHECK_EQ(static_cast<int>(portNames.size()),N_PORTS);
    for(const auto &it : portNames) {
        CHECK_TRUE(dut->isNamedPort(it));
        if (std::strcmp(it.c_str(),"input_port") == 0) continue;
        if (std::strcmp(it.c_str(),"output_port") == 0) continue;
        if (std::strcmp(it.c_str(),"inout_port") == 0) continue;
        if (std::strcmp(it.c_str(),"array_port") == 0) continue;
        if (std::strcmp(it.c_str(),"wide_port") == 0) continue;
        if (std::strcmp(it.c_str(),"wide_array_port") == 0) continue;
        if (std::strcmp(it.c_str(),"logic_port") == 0) continue;
        if (std::strcmp(it.c_str(),"reg_port") == 0) continue;
        if (std::strcmp(it.c_str(),"wire_port") == 0) continue;
        if (std::strcmp(it.c_str(),"_abcdefghijklmnopqrstuvwxyz_ABCDEFGHIJKLNMNOPQRSTUVWXYZ_1234567890_port") == 0) continue;
        output->fatal(CALL_INFO,-1,"Unknown port name returned: %s\n",it.c_str());
    }
}

void testGetPortDirection(SST::Output * output, SST::VerilatorSST::VerilatorSSTBase * dut){
    SKIP_TEST; //TODO https://github.com/tactcomplabs/verilator-sst/issues/13
    VPortType direction;
    CHECK_TRUE(dut->getPortType("input_port", direction));
    CHECK_EQ(direction,VPortType::V_INPUT);
    CHECK_TRUE(dut->getPortType("output_port", direction));
    CHECK_EQ(direction,VPortType::V_OUTPUT);
    CHECK_TRUE(dut->getPortType("inout_port", direction));
    // CHECK_EQ(direction,VPortType::V_INOUT);
}

void testGetPortWidth(SST::Output * output, SST::VerilatorSST::VerilatorSSTBase * dut){
    unsigned width;
    CHECK_TRUE(dut->getPortWidth("wide_port",width));
    CHECK_EQ(width,8888);
    CHECK_TRUE(dut->getPortWidth("array_port",width));
    CHECK_EQ(width,1);
    CHECK_TRUE(dut->getPortWidth("wide_array_port",width));
    CHECK_EQ(width,8888);
}

void testGetPortDepth(SST::Output * output, SST::VerilatorSST::VerilatorSSTBase * dut){
    unsigned depth;
    CHECK_TRUE(dut->getPortDepth("wide_port",depth));
    CHECK_EQ(depth,1);
    CHECK_TRUE(dut->getPortDepth("array_port",depth));
    CHECK_EQ(depth,9999);
    CHECK_TRUE(dut->getPortDepth("wide_array_port",depth));
    CHECK_EQ(depth,9999);
}

void PortTestBench::runTestSuite(SST::Cycle_t){
    testIsNamedPort(output, dut);
    testNumPorts(output, dut);
    testGetPortNames(output, dut);
    testGetPortDirection(output, dut);
    testGetPortWidth(output, dut);
    testGetPortDepth(output, dut);
}
