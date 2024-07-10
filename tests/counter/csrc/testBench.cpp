#include "SST.h"
#include "VerilatorTestDirect.h"

using namespace SST::VerilatorSST;

#define STOP_WIDTH 4
#define N_PORTS 4
#define CHECK_EQ(got, exp) \
    if ((got) != (exp)) { \
        output.fatal(CALL_INFO,-1,"GOT = %u EXP = %u\n",got,exp); \
    }

void VerilatorTestDirect::testPortNames() {
    const auto portNames = model->getPortsNames();
    CHECK_EQ(static_cast<int>(portNames.size()),N_PORTS);
    for(const auto &it : portNames) {
        CHECK_EQ(model->isNamedPort(it),true);
        if (std::strcmp(it.c_str(),"clk") == 0) continue;
        if (std::strcmp(it.c_str(),"reset_l") == 0) continue;
        if (std::strcmp(it.c_str(),"stop") == 0) continue;
        if (std::strcmp(it.c_str(),"done") == 0) continue;
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
    CHECK_EQ(model->getPortWidth("reset_l",width), true);
    CHECK_EQ(width, 1);
    CHECK_EQ(model->getPortWidth("stop",width), true);
    CHECK_EQ(width, STOP_WIDTH);
    CHECK_EQ(model->getPortWidth("done",width), true);
    CHECK_EQ(width, 1);
}

void VerilatorTestDirect::testPortDepth() {
    unsigned depth;
    CHECK_EQ(model->getPortDepth("clk",depth), true);
    CHECK_EQ(depth, 1);
    CHECK_EQ(model->getPortDepth("reset_l",depth), true);
    CHECK_EQ(depth, 1);
    CHECK_EQ(model->getPortDepth("stop",depth), true);
    CHECK_EQ(depth, 1);
    CHECK_EQ(model->getPortDepth("done",depth), true);
    CHECK_EQ(depth, 1);
}

void VerilatorTestDirect::testPortType() {
    VPortType portType;
    CHECK_EQ(model->getPortType("clk",portType), true);
    CHECK_EQ(portType,VPortType::V_INPUT);
    CHECK_EQ(model->getPortType("reset_l",portType), true);
    CHECK_EQ(portType,VPortType::V_INPUT);
    CHECK_EQ(model->getPortType("stop",portType), true);
    CHECK_EQ(portType,VPortType::V_INPUT);
    CHECK_EQ(model->getPortType("done",portType), true);
    CHECK_EQ(portType,VPortType::V_OUTPUT);
}

void VerilatorTestDirect::testPortWrite() {
    const std::vector<uint8_t> high = {1};
    std::vector<uint8_t> stop;
    stop.resize((STOP_WIDTH+7)/8);
    model->writePort("clk", high);
    model->writePort("reset_l", high);
    model->writePort("stop", stop);
}

void VerilatorTestDirect::testPortRead() {
    std::vector<uint8_t> done = model->readPort("done");
    CHECK_EQ(static_cast<uint32_t>(done.size()),1);
}

void VerilatorTestDirect::testPortWriteRead() {
    // SKIPPED
}
