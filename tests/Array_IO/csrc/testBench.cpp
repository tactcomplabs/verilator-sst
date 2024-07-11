#include "VerilatorTestDirect.h"

using namespace SST::VerilatorSST;

#define PACKED_WIDTH 24
#define UNPACKED_ELEMENTS 10
#define N_PORTS 2
#define CHECK_EQ(got, exp) \
    if ((got) != (exp)) { \
        output.fatal(CALL_INFO,-1,"GOT = %u EXP = %u\n",got,exp); \
    }

void VerilatorTestDirect::testPortNames() {
    const std::vector<std::string> & portNames = model->getPortsNames();
    CHECK_EQ(static_cast<int>(portNames.size()),N_PORTS);
    for(const auto &it : portNames) {
        CHECK_EQ(model->isNamedPort(it),true);
        if (std::strcmp(it.c_str(),"arr_in") == 0) continue;
        if (std::strcmp(it.c_str(),"arr_out") == 0) continue;
        output.fatal(CALL_INFO,-1,"Unknown port name returned\n");
    }
}

void VerilatorTestDirect::testNumPorts() {
    CHECK_EQ(model->getNumPorts(),N_PORTS);
}

void VerilatorTestDirect::testPortWidth() {
    unsigned width;
    CHECK_EQ(model->getPortWidth("arr_in",width), true);
    CHECK_EQ(width, PACKED_WIDTH);
    assert(model->getPortWidth("arr_out",width));
    CHECK_EQ(width, PACKED_WIDTH);
}

void VerilatorTestDirect::testPortDepth() {
    unsigned elements;
    CHECK_EQ(model->getPortDepth("arr_in",elements), true);
    CHECK_EQ(elements, UNPACKED_ELEMENTS);
    CHECK_EQ(model->getPortDepth("arr_out",elements), true);
    CHECK_EQ(elements, UNPACKED_ELEMENTS);
}

void VerilatorTestDirect::testPortType() {
    VPortType portType;
    CHECK_EQ(model->getPortType("arr_in",portType), true);
    CHECK_EQ(portType,VPortType::V_INPUT);
    CHECK_EQ(model->getPortType("arr_out",portType), true);
    CHECK_EQ(portType,VPortType::V_OUTPUT);
}

void VerilatorTestDirect::testPortWrite() {
    std::vector<uint8_t> writeVals;
    const uint32_t ngroups = ((PACKED_WIDTH + 7)/ 8)*UNPACKED_ELEMENTS;
    writeVals.resize(ngroups);
    model->writePort("arr_in", writeVals);
}

void VerilatorTestDirect::testPortRead() {
    std::vector<uint8_t> readVals = model->readPort("arr_out");
    const uint32_t ngroups = ((PACKED_WIDTH + 7)/ 8)*UNPACKED_ELEMENTS;
    CHECK_EQ(static_cast<uint32_t>(readVals.size()),ngroups);
}

void VerilatorTestDirect::testPortWriteRead() {
    std::vector<uint8_t> writeVals;
    const uint32_t ngroups = ((PACKED_WIDTH + 7)/ 8)*UNPACKED_ELEMENTS;
    writeVals.resize(ngroups);
    for (auto & it : writeVals) {
        it = static_cast<uint8_t>(rand());
    }
    model->writePort("arr_in", writeVals);
    model->forceEvaluation();

    std::vector<uint8_t> expectedVals;
    expectedVals.resize(ngroups);
    for (size_t i = 0; i < ngroups; i++) {
        expectedVals[i] = ~writeVals[i];
    }
    std::vector<uint8_t> readVals = model->readPort("arr_out");
    const uint8_t * readData = reinterpret_cast<uint8_t*>(readVals.data());
    const uint8_t * expectedData = reinterpret_cast<uint8_t*>(expectedVals.data());
    CHECK_EQ(std::memcmp(readData,expectedData,ngroups),0);
}
