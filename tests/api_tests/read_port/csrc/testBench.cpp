#define READ_QUAD_PORT_MSB_EXP 0x0123456789ABCDEF
#define READ_QUAD_PORT_LSB_EXP 0xFEDCBA9876543210
#define READ_PORT_EXP 0x1

#include "testHarness.h"

using namespace SST::VerilatorSST;

void testReadPort(SST::Output * output, SST::VerilatorSST::VerilatorSSTBase * dut){
    //TODO https://github.com/tactcomplabs/verilator-sst/issues/15
    const std::vector<uint8_t> & read_port_v = dut->readPort("read_port");
    CHECK_EQ(read_port_v[0] & 1, READ_PORT_EXP);
}

void testReadQuadPort(SST::Output * output, SST::VerilatorSST::VerilatorSSTBase * dut){
    const std::vector<uint8_t> & read_port_v = dut->readPort("read_quad_port");
    uint64_t read_quad_port_msb;
    uint64_t read_quad_port_lsb;
    memcpy(&read_quad_port_msb, read_port_v.data()+8, sizeof(uint64_t));
    memcpy(&read_quad_port_lsb, read_port_v.data(), sizeof(uint64_t));
    CHECK_EQ(read_quad_port_msb, READ_QUAD_PORT_MSB_EXP);
    CHECK_EQ(read_quad_port_lsb, READ_QUAD_PORT_LSB_EXP);
}

void testReadArrayPort(SST::Output * output, SST::VerilatorSST::VerilatorSSTBase * dut){
    const std::vector<uint8_t> & read_port_v = dut->readPort("read_array_port");
    for(int i = 0; i < 128; i++) {
        CHECK_EQ(read_port_v[i] & 1, i % 2);
    }
}

void testReadQuadArrayPort(SST::Output * output, SST::VerilatorSST::VerilatorSSTBase * dut){
    const std::vector<uint8_t> & read_quad_array_port_v = dut->readPort("read_quad_array_port");
    uint64_t quad_msb;
    uint64_t quad_lsb;
    const uint64_t mask = 0xFFFFFFFFFFFFFF00;
    for(uint16_t i = 0; i < 128; i++) {
        memcpy(&quad_msb, read_quad_array_port_v.data()+(16*i)+8, sizeof(uint64_t));
        memcpy(&quad_lsb, read_quad_array_port_v.data()+(16*i),   sizeof(uint64_t));
        CHECK_EQ(quad_msb, (READ_QUAD_PORT_MSB_EXP & mask) | i);
        CHECK_EQ(quad_lsb, (READ_QUAD_PORT_LSB_EXP & mask) | i);
    }
}

void testReadRegPort(SST::Output * output, SST::VerilatorSST::VerilatorSSTBase * dut){
    const std::vector<uint8_t> & read_reg_port_v = dut->readPort("read_reg_port");
    CHECK_EQ(read_reg_port_v[0],READ_PORT_EXP);
}

void testReadWirePort(SST::Output * output, SST::VerilatorSST::VerilatorSSTBase * dut){
    const std::vector<uint8_t> & read_wire_port_v = dut->readPort("read_wire_port");
    CHECK_EQ(read_wire_port_v[0],READ_PORT_EXP);
}

void TestHarness::runTestSuite(SST::Cycle_t cycle){
    testReadPort(output, dut);
    testReadQuadPort(output,dut);
    testReadArrayPort(output, dut);
    testReadQuadArrayPort(output, dut);
    testReadRegPort(output, dut);
    testReadWirePort(output, dut);
}
