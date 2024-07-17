#include "WritePortTestBench.h"

using namespace SST::VerilatorSST;

void testWritePort(SST::Output * output, SST::VerilatorSST::VerilatorSSTBase * dut){
    //TODO https://github.com/tactcomplabs/verilator-sst/issues/16
    const std::vector<uint8_t> write_port_v = {1};
    dut->writePort("write_port", write_port_v);
    dut->clock(-1);
    const std::vector<uint8_t> read_write_port_v = dut->readPort("write_port");
    CHECK_EQ(read_write_port_v.size(),1);
    CHECK_EQ(read_write_port_v[0],read_write_port_v[0]);
}

void testWriteQuadPort(SST::Output * output, SST::VerilatorSST::VerilatorSSTBase * dut){
    // const std::vector<uint8_t> & read_port_v = dut->readPort("read_quad_port");
    // uint64_t read_quad_port_msb;
    // uint64_t read_quad_port_lsb;
    // memcpy(&read_quad_port_msb, read_port_v.data()+8, sizeof(uint64_t));
    // memcpy(&read_quad_port_lsb, read_port_v.data(), sizeof(uint64_t));
    // CHECK_EQ(read_quad_port_msb, READ_QUAD_PORT_MSB_EXP);
    // CHECK_EQ(read_quad_port_lsb, READ_QUAD_PORT_LSB_EXP);
}

void testWriteArrayPort(SST::Output * output, SST::VerilatorSST::VerilatorSSTBase * dut){
    // const std::vector<uint8_t> & read_port_v = dut->readPort("read_array_port");
    // for(int i = 0; i < 128; i++) {
    //     CHECK_EQ(read_port_v[i] & 1, i % 2);
    // }
}

void testWriteQuadArrayPort(SST::Output * output, SST::VerilatorSST::VerilatorSSTBase * dut){
    // const std::vector<uint8_t> & read_quad_array_port_v = dut->readPort("read_quad_array_port");
    // uint64_t quad_msb;
    // uint64_t quad_lsb;
    // const uint64_t mask = 0xFFFFFFFFFFFFFF00;
    // for(uint16_t i = 0; i < 128; i++) {
    //     memcpy(&quad_msb, read_quad_array_port_v.data()+(16*i)+8, sizeof(uint64_t));
    //     memcpy(&quad_lsb, read_quad_array_port_v.data()+(16*i),   sizeof(uint64_t));
    //     CHECK_EQ(quad_msb, (READ_QUAD_PORT_MSB_EXP & mask) | i);
    //     CHECK_EQ(quad_lsb, (READ_QUAD_PORT_LSB_EXP & mask) | i);
    // }
}

void testWriteRegPort(SST::Output * output, SST::VerilatorSST::VerilatorSSTBase * dut){
    // const std::vector<uint8_t> & read_reg_port_v = dut->readPort("read_reg_port");
    // CHECK_EQ(read_reg_port_v[0],READ_PORT_EXP);
}

void testWriteWirePort(SST::Output * output, SST::VerilatorSST::VerilatorSSTBase * dut){
    // const std::vector<uint8_t> & read_wire_port_v = dut->readPort("read_wire_port");
    // CHECK_EQ(read_wire_port_v[0],READ_PORT_EXP);
}

void WritePortTestBench::runTestSuite(SST::Cycle_t cycle){
    testWritePort(output, dut);
    testWriteQuadPort(output,dut);
    testWriteArrayPort(output, dut);
    testWriteQuadArrayPort(output, dut);
    testWriteRegPort(output, dut);
    testWriteWirePort(output, dut);
}
