#include "SST.h"
#include "VerilatorTestDirect.h"

using namespace SST::VerilatorSST;

#define ACCUM_WIDTH 64
#define ACCUM_DEPTH 1
#define ADD_WIDTH 32
#define ADD_DEPTH 1
#define N_PORTS 6
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
        if (std::strcmp(it.c_str(),"reset_l") == 0) continue;
        if (std::strcmp(it.c_str(),"en") == 0) continue;
        if (std::strcmp(it.c_str(),"add") == 0) continue;
        if (std::strcmp(it.c_str(),"accum") == 0) continue;
        if (std::strcmp(it.c_str(),"done") == 0) continue;
        output.fatal(CALL_INFO,-1,"Unknown port name returned: %s\n",it.c_str());
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
    CHECK_EQ(model->getPortWidth("en",width), true);
    CHECK_EQ(width, 1);
    CHECK_EQ(model->getPortWidth("add",width), true);
    CHECK_EQ(width, ADD_WIDTH);
    CHECK_EQ(model->getPortWidth("accum",width), true);
    CHECK_EQ(width, ACCUM_WIDTH);
    CHECK_EQ(model->getPortWidth("done",width), true);
    CHECK_EQ(width, 1);
}

void VerilatorTestDirect::testPortDepth() {
    unsigned depth;
    CHECK_EQ(model->getPortDepth("clk",depth), true);
    CHECK_EQ(depth, 1);
    CHECK_EQ(model->getPortDepth("reset_l",depth), true);
    CHECK_EQ(depth, 1);
    CHECK_EQ(model->getPortDepth("en",depth), true);
    CHECK_EQ(depth, 1);
    CHECK_EQ(model->getPortDepth("add",depth), true);
    CHECK_EQ(depth, ADD_DEPTH);
    CHECK_EQ(model->getPortDepth("accum",depth), true);
    CHECK_EQ(depth, ACCUM_DEPTH);
    CHECK_EQ(model->getPortDepth("done",depth), true);
    CHECK_EQ(depth, 1);
}

void VerilatorTestDirect::testPortType() {
    VPortType portType;
    CHECK_EQ(model->getPortType("clk",portType), true);
    CHECK_EQ(portType, VPortType::V_INPUT);
    CHECK_EQ(model->getPortType("reset_l",portType), true);
    CHECK_EQ(portType, VPortType::V_INPUT);
    CHECK_EQ(model->getPortType("en",portType), true);
    CHECK_EQ(portType, VPortType::V_INPUT);
    CHECK_EQ(model->getPortType("add",portType), true);
    CHECK_EQ(portType, VPortType::V_INPUT);
    CHECK_EQ(model->getPortType("accum",portType), true);
    CHECK_EQ(portType, VPortType::V_OUTPUT);
    CHECK_EQ(model->getPortType("done",portType), true);
    CHECK_EQ(portType, VPortType::V_OUTPUT);
}

void VerilatorTestDirect::testPortWrite() {
    const std::vector<uint8_t> high = {1};
    std::vector<uint8_t> add;
    add.resize(((ADD_WIDTH+7)/8)*ADD_DEPTH);
    model->writePort("clk", high);
    model->writePort("reset_l", high);
    model->writePort("en", high);
    model->writePort("add", add);
}

void VerilatorTestDirect::testPortRead() {
    const std::vector<uint8_t> accum = model->readPort("accum");
    CHECK_EQ(static_cast<uint32_t>(accum.size()),((ACCUM_WIDTH+7)/8)*ACCUM_DEPTH);
    const std::vector<uint8_t> done = model->readPort("done");
    CHECK_EQ(static_cast<uint32_t>(done.size()),1);
}

void VerilatorTestDirect::testPortWriteRead() {
    // SKIPPED
}
