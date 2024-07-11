#include "SST.h"
#include "VerilatorTestDirect.h"

using namespace SST::VerilatorSST;

#define CHUNK_SIZE 4
#define NUM_CHUNKS 16
#define SCRATCHPAD_BASE 16
#define N_PORTS 7
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
        if (std::strcmp(it.c_str(),"en") == 0) continue;
        if (std::strcmp(it.c_str(),"write") == 0) continue;
        if (std::strcmp(it.c_str(),"addr") == 0) continue;
        if (std::strcmp(it.c_str(),"len") == 0) continue;
        if (std::strcmp(it.c_str(),"wdata") == 0) continue;
        if (std::strcmp(it.c_str(),"rdata") == 0) continue;
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
    CHECK_EQ(model->getPortWidth("en",width), true);
    CHECK_EQ(width, 1);
    CHECK_EQ(model->getPortWidth("write",width), true);
    CHECK_EQ(width, 1);
    CHECK_EQ(model->getPortWidth("addr",width), true);
    CHECK_EQ(width, 64);
    CHECK_EQ(model->getPortWidth("len",width), true);
    CHECK_EQ(width, 2);
    CHECK_EQ(model->getPortWidth("wdata",width), true);
    CHECK_EQ(width, 64);
    CHECK_EQ(model->getPortWidth("rdata",width), true);
    CHECK_EQ(width, 64);
}

void VerilatorTestDirect::testPortDepth() {
    unsigned depth;
    CHECK_EQ(model->getPortDepth("clk",depth), true);
    CHECK_EQ(depth, 1);
    CHECK_EQ(model->getPortDepth("en",depth), true);
    CHECK_EQ(depth, 1);
    CHECK_EQ(model->getPortDepth("write",depth), true);
    CHECK_EQ(depth, 1);
    CHECK_EQ(model->getPortDepth("addr",depth), true);
    CHECK_EQ(depth, 1);
    CHECK_EQ(model->getPortDepth("len",depth), true);
    CHECK_EQ(depth, 1);
    CHECK_EQ(model->getPortDepth("wdata",depth), true);
    CHECK_EQ(depth, 1);
    CHECK_EQ(model->getPortDepth("rdata",depth), true);
    CHECK_EQ(depth, 1);
}

void VerilatorTestDirect::testPortType() {
    VPortType portType;
    CHECK_EQ(model->getPortType("clk",portType), true);
    CHECK_EQ(portType, VPortType::V_INPUT);
    CHECK_EQ(model->getPortType("en",portType), true);
    CHECK_EQ(portType, VPortType::V_INPUT);
    CHECK_EQ(model->getPortType("write",portType), true);
    CHECK_EQ(portType, VPortType::V_INPUT);
    CHECK_EQ(model->getPortType("addr",portType), true);
    CHECK_EQ(portType, VPortType::V_INPUT);
    CHECK_EQ(model->getPortType("len",portType), true);
    CHECK_EQ(portType, VPortType::V_INPUT);
    CHECK_EQ(model->getPortType("wdata",portType), true);
    CHECK_EQ(portType, VPortType::V_INPUT);
    CHECK_EQ(model->getPortType("rdata",portType), true);
    CHECK_EQ(portType, VPortType::V_OUTPUT);
}

void VerilatorTestDirect::testPortWrite() {
    const std::vector<uint8_t> high = {1};
    std::vector<uint8_t> data;
    data.resize(8);
    model->writePort("clk", high);
    model->writePort("en", high);
    model->writePort("write", high);
    model->writePort("addr", data);
    model->writePort("wdata", data);
}

void VerilatorTestDirect::testPortRead() {
    const std::vector<uint8_t> rdata = model->readPort("rdata");
    CHECK_EQ(static_cast<uint32_t>(rdata.size()),8);
}

void VerilatorTestDirect::testPortWriteRead() {
    // SKIPPED
}
