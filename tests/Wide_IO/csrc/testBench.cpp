#include "VerilatorTestDirect.h"

using namespace SST::VerilatorSST;

#define LONG_WIDTH 64
#define LONG_LONG_WIDTH 128
#define SUPER_WIDTH 512
#define N_PORTS 6
#define CHECK_EQ(got, exp) \
    if ((got) != (exp)) { \
        output.fatal(CALL_INFO,-1,"GOT = %u EXP = %u\n",got,exp); \
    }

void VerilatorTestDirect::testPortNames() {
    const auto portNames = model->getPortsNames();
    CHECK_EQ(static_cast<int>(portNames.size()),N_PORTS);
    for(const auto &it : portNames) {
        CHECK_EQ(model->isNamedPort(it),true);
        if (std::strcmp(it.c_str(),"long_in") == 0) continue;
        if (std::strcmp(it.c_str(),"long_long_in") == 0) continue;
        if (std::strcmp(it.c_str(),"super_in") == 0) continue;
        if (std::strcmp(it.c_str(),"long_out") == 0) continue;
        if (std::strcmp(it.c_str(),"long_long_out") == 0) continue;
        if (std::strcmp(it.c_str(),"super_out") == 0) continue;
        output.fatal(CALL_INFO,-1,"Unknown port name returned\n");
    }
}

void VerilatorTestDirect::testNumPorts() {
    CHECK_EQ(model->getNumPorts(),N_PORTS);
}

void VerilatorTestDirect::testPortWidth() {
    unsigned width;
    CHECK_EQ(model->getPortWidth("long_in",width), true);
    CHECK_EQ(width, LONG_WIDTH);
    CHECK_EQ(model->getPortWidth("long_long_in",width), true);
    CHECK_EQ(width, LONG_LONG_WIDTH);
    CHECK_EQ(model->getPortWidth("super_in",width), true);
    CHECK_EQ(width, SUPER_WIDTH);
    CHECK_EQ(model->getPortWidth("long_out",width), true);
    CHECK_EQ(width, LONG_WIDTH);
    CHECK_EQ(model->getPortWidth("long_long_out",width), true);
    CHECK_EQ(width, LONG_LONG_WIDTH);
    CHECK_EQ(model->getPortWidth("super_out",width), true);
    CHECK_EQ(width, SUPER_WIDTH);

}

void VerilatorTestDirect::testPortDepth() {
    unsigned depth;
    CHECK_EQ(model->getPortDepth("long_in",depth), true);
    CHECK_EQ(depth, 1);
    CHECK_EQ(model->getPortDepth("long_long_in",depth), true);
    CHECK_EQ(depth, 1);
    CHECK_EQ(model->getPortDepth("super_in",depth), true);
    CHECK_EQ(depth, 1);
    CHECK_EQ(model->getPortDepth("long_out",depth), true);
    CHECK_EQ(depth, 1);
    CHECK_EQ(model->getPortDepth("long_long_out",depth), true);
    CHECK_EQ(depth, 1);
    CHECK_EQ(model->getPortDepth("super_out",depth), true);
    CHECK_EQ(depth, 1);
}

void VerilatorTestDirect::testPortType() {
    VPortType portType;
    CHECK_EQ(model->getPortType("long_in",portType), true);
    CHECK_EQ(portType, VPortType::V_INPUT);
    CHECK_EQ(model->getPortType("long_long_in",portType), true);
    CHECK_EQ(portType, VPortType::V_INPUT);
    CHECK_EQ(model->getPortType("super_in",portType), true);
    CHECK_EQ(portType, VPortType::V_INPUT);
    CHECK_EQ(model->getPortType("long_out",portType), true);
    CHECK_EQ(portType, VPortType::V_OUTPUT);
    CHECK_EQ(model->getPortType("long_long_out",portType), true);
    CHECK_EQ(portType, VPortType::V_OUTPUT);
    CHECK_EQ(model->getPortType("super_out",portType), true);
    CHECK_EQ(portType, VPortType::V_OUTPUT);
}

void VerilatorTestDirect::testPortWrite() {
    std::vector<uint8_t> longVal;
    std::vector<uint8_t> longLongVal;
    std::vector<uint8_t> superVal;
    longVal.resize((LONG_WIDTH + 7)/ 8);
    longLongVal.resize((LONG_LONG_WIDTH + 7)/ 8);
    superVal.resize((SUPER_WIDTH + 7)/ 8);
    model->writePort("long_in",longVal);
    model->writePort("long_long_in",longLongVal);
    model->writePort("super_in",superVal);
}

void VerilatorTestDirect::testPortRead() {
    const auto longVal = model->readPort("long_in");
    const auto longLongVal = model->readPort("long_long_in");
    const auto superVal = model->readPort("super_in");
    CHECK_EQ(static_cast<uint32_t>(longVal.size()),(LONG_WIDTH + 7)/ 8);
    CHECK_EQ(static_cast<uint32_t>(longLongVal.size()),(LONG_LONG_WIDTH + 7)/ 8);
    CHECK_EQ(static_cast<uint32_t>(superVal.size()),(SUPER_WIDTH + 7)/ 8);
}

void VerilatorTestDirect::testPortWriteRead() {
    const auto longBytes = (LONG_WIDTH + 7)/ 8;
    const auto longLongBytes = (LONG_LONG_WIDTH + 7)/ 8;
    const auto superBytes = (SUPER_WIDTH + 7)/ 8;

    // generate random write data
    std::vector<uint8_t> writeLongVal;
    std::vector<uint8_t> writeLongLongVal;
    std::vector<uint8_t> writeSuperVal;
    writeLongVal.resize(longBytes);
    writeLongLongVal.resize(longLongBytes);
    writeSuperVal.resize(superBytes);
    for(auto &it : writeLongVal) {
        it = static_cast<uint8_t>(rand());
    }
    for(auto &it : writeLongLongVal) {
        it = static_cast<uint8_t>(rand());
    }
    for(auto &it : writeSuperVal) {
        it = static_cast<uint8_t>(rand());
    }
    model->writePort("long_in",writeLongVal);
    model->writePort("long_long_in",writeLongLongVal);
    model->writePort("super_in",writeSuperVal);
    model->forceEvaluation();

    // calculate expected read data
    std::vector<uint8_t> expectedLongVal;
    std::vector<uint8_t> expectedLongLongVal;
    std::vector<uint8_t> expectedSuperVal;
    expectedLongVal.resize(longBytes);
    expectedLongLongVal.resize(longLongBytes);
    expectedSuperVal.resize(superBytes);
    for(auto i = 0; i < longBytes; i++) {
        expectedLongVal[i] = ~writeLongVal[i];
    }
    for(auto i = 0; i < longLongBytes; i++) {
        expectedLongLongVal[i] = ~writeLongLongVal[i];
    }
    for(auto i = 0; i < superBytes; i++) {
        expectedSuperVal[i] = ~writeSuperVal[i];
    }

    // check value match
    const auto readLongVal = model->readPort("long_out");
    const auto readLongLongVal = model->readPort("long_long_out");
    const auto readSuperVal = model->readPort("super_out");
    CHECK_EQ(std::memcmp(readLongVal.data(),expectedLongVal.data(),longBytes),0);
    CHECK_EQ(std::memcmp(readLongLongVal.data(),expectedLongLongVal.data(),longLongBytes),0);
    CHECK_EQ(std::memcmp(readSuperVal.data(),expectedSuperVal.data(),superBytes),0);
}
