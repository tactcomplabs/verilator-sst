#include "SST.h"
#include "VerilatorTestDirect.h"

using namespace SST::VerilatorSST;


#define MEM_DEBUG_WIDTH 8
#define MEM_DEBUG_DEPTH (1 << 16)
#define N_PORTS 5
#define CHECK_EQ(got, exp) \
    if ((got) != (exp)) { \
        output.fatal(CALL_INFO,-1,"GOT = %u EXP = %u\n",got,exp); \
    }

void VerilatorTestDirect::testPortNames() {
    const std::vector<std::string> & portNames = model->getPortsNames();
    CHECK_EQ(static_cast<int>(portNames.size()),N_PORTS);
    for(const auto &it : portNames) {
        CHECK_EQ(model->isNamedPort(it),true);
        if (std::strcmp(it.c_str(),"clk") == 0) continue;
        if (std::strcmp(it.c_str(),"rst_l") == 0) continue;
        if (std::strcmp(it.c_str(),"RX") == 0) continue;
        if (std::strcmp(it.c_str(),"TX") == 0) continue;
        if (std::strcmp(it.c_str(),"mem_debug") == 0) continue;
        output.fatal(CALL_INFO,-1,"Unknown port name returned\n");
    }
}

void VerilatorTestDirect::testNumPorts() {
    CHECK_EQ(model->getNumPorts(),N_PORTS);
}

void VerilatorTestDirect::testPortWidth() {
    unsigned width;
    CHECK_EQ(model->getPortWidth("clk",width), true);
    CHECK_EQ(width, 1);
    CHECK_EQ(model->getPortWidth("rst_l",width), true);
    CHECK_EQ(width, 1);
    CHECK_EQ(model->getPortWidth("RX",width), true);
    CHECK_EQ(width, 1);
    CHECK_EQ(model->getPortWidth("TX",width), true);
    CHECK_EQ(width, 1);
    CHECK_EQ(model->getPortWidth("mem_debug",width), true);
    CHECK_EQ(width, MEM_DEBUG_WIDTH);
}

void VerilatorTestDirect::testPortDepth() {
    unsigned depth;
    CHECK_EQ(model->getPortDepth("clk",depth), true);
    CHECK_EQ(depth, 1);
    CHECK_EQ(model->getPortDepth("rst_l",depth), true);
    CHECK_EQ(depth, 1);
    CHECK_EQ(model->getPortDepth("RX",depth), true);
    CHECK_EQ(depth, 1);
    CHECK_EQ(model->getPortDepth("TX",depth), true);
    CHECK_EQ(depth, 1);
    CHECK_EQ(model->getPortDepth("mem_debug",depth), true);
    CHECK_EQ(depth, MEM_DEBUG_DEPTH);
}

void VerilatorTestDirect::testPortType() {
    VPortType portType;
    CHECK_EQ(model->getPortType("clk",portType), true);
    CHECK_EQ(portType, VPortType::V_INPUT);
    CHECK_EQ(model->getPortType("rst_l",portType), true);
    CHECK_EQ(portType, VPortType::V_INPUT);
    CHECK_EQ(model->getPortType("RX",portType), true);
    CHECK_EQ(portType, VPortType::V_INPUT);
    CHECK_EQ(model->getPortType("TX",portType), true);
    CHECK_EQ(portType, VPortType::V_OUTPUT);
    CHECK_EQ(model->getPortType("mem_debug",portType), true);
    CHECK_EQ(portType, VPortType::V_OUTPUT);
}

void VerilatorTestDirect::testPortWrite() {
    const std::vector<uint8_t> high = {1};
    model->writePort("clk", high);
    model->writePort("rst_l", high);
    model->writePort("RX", high);
}

void VerilatorTestDirect::testPortRead() {
    const std::vector<uint8_t> mem_debug = model->readPort("mem_debug");
    CHECK_EQ(static_cast<uint32_t>(mem_debug.size()),((MEM_DEBUG_WIDTH+7)/8)*MEM_DEBUG_DEPTH);
    const std::vector<uint8_t> tx = model->readPort("TX");
    CHECK_EQ(static_cast<uint32_t>(tx.size()),1);
}

void VerilatorTestDirect::testPortWriteRead() {
    // SKIPPED
}
