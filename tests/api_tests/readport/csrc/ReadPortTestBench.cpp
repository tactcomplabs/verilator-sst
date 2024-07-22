// tests/api_tests/readport/csrc/ReadPortTestBench.cpp
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details

#define READ_QUAD_PORT_MSB_EXP 0x0123456789ABCDEF
#define READ_QUAD_PORT_LSB_EXP 0xFEDCBA9876543210
#define READ_PORT_EXP 0x1

#include "ReadPortTestBench.h"

using namespace SST::VerilatorSST;

void testReadPort(SST::Output * output, SST::VerilatorSST::VerilatorSSTBase * dut){
    const std::vector<uint8_t> & read_port_v = dut->readPort("read_port");
    CHECK_EQ(read_port_v.size(), 1);
    CHECK_EQ(read_port_v[0] & 1, READ_PORT_EXP);
}

void testReadBytePort(SST::Output * output, SST::VerilatorSST::VerilatorSSTBase * dut){
    const std::vector<uint8_t> & read_byte_port_v = dut->readPort("read_byte_port");
    CHECK_EQ(read_byte_port_v.size(), 1);
    CHECK_EQ(read_byte_port_v[0], static_cast<uint8_t>(READ_QUAD_PORT_MSB_EXP));
}

void testReadHalfPort(SST::Output * output, SST::VerilatorSST::VerilatorSSTBase * dut){
    const std::vector<uint8_t> & read_half_port_v = dut->readPort("read_half_port");
    CHECK_EQ(read_half_port_v.size(), 2);
    uint16_t read_half_port;
    memcpy(&read_half_port, read_half_port_v.data(), sizeof(uint16_t));
    CHECK_EQ(read_half_port, static_cast<uint16_t>(READ_QUAD_PORT_MSB_EXP));
}

void testReadWordPort(SST::Output * output, SST::VerilatorSST::VerilatorSSTBase * dut){
    const std::vector<uint8_t> & read_word_port_v = dut->readPort("read_word_port");
    CHECK_EQ(read_word_port_v.size(), 4);
    uint32_t read_word_port;
    memcpy(&read_word_port, read_word_port_v.data(), sizeof(uint32_t));
    CHECK_EQ(read_word_port, static_cast<uint32_t>(READ_QUAD_PORT_MSB_EXP));
}

void testReadDoublePort(SST::Output * output, SST::VerilatorSST::VerilatorSSTBase * dut){
    const std::vector<uint8_t> & read_double_port_v = dut->readPort("read_double_port");
    CHECK_EQ(read_double_port_v.size(), 8);
    uint64_t read_double_port;
    memcpy(&read_double_port, read_double_port_v.data(), sizeof(uint64_t));
    CHECK_EQ(read_double_port, static_cast<uint32_t>(READ_QUAD_PORT_MSB_EXP));
}

void testReadQuadPort(SST::Output * output, SST::VerilatorSST::VerilatorSSTBase * dut){
    const std::vector<uint8_t> & read_quad_port_v = dut->readPort("read_quad_port");
    CHECK_EQ(read_quad_port_v.size(), 16);
    uint64_t read_quad_port_msb;
    uint64_t read_quad_port_lsb;
    memcpy(&read_quad_port_msb, read_quad_port_v.data()+8, sizeof(uint64_t));
    memcpy(&read_quad_port_lsb, read_quad_port_v.data(), sizeof(uint64_t));
    CHECK_EQ(read_quad_port_msb, READ_QUAD_PORT_MSB_EXP);
    CHECK_EQ(read_quad_port_lsb, READ_QUAD_PORT_LSB_EXP);
}

void testReadArrayPort(SST::Output * output, SST::VerilatorSST::VerilatorSSTBase * dut){
    const std::vector<uint8_t> & read_array_port_v = dut->readPort("read_array_port");
    CHECK_EQ(read_array_port_v.size(), 128);
    for(int i = 0; i < 128; i++) {
        CHECK_EQ(read_array_port_v[i] & 1, i % 2);
    }
}

void testReadQuadArrayPort(SST::Output * output, SST::VerilatorSST::VerilatorSSTBase * dut){
    const std::vector<uint8_t> & read_quad_array_port_v = dut->readPort("read_quad_array_port");
    CHECK_EQ(read_quad_array_port_v.size(), 16*128);
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
    CHECK_EQ(read_reg_port_v.size(), 1);
    CHECK_EQ(read_reg_port_v[0],READ_PORT_EXP);
}

void testReadWirePort(SST::Output * output, SST::VerilatorSST::VerilatorSSTBase * dut){
    const std::vector<uint8_t> & read_wire_port_v = dut->readPort("read_wire_port");
    CHECK_EQ(read_wire_port_v.size(), 1);
    CHECK_EQ(read_wire_port_v[0],READ_PORT_EXP);
}

void ReadPortTestBench::runTestSuite(SST::Cycle_t){
    testReadPort(output, dut);
    testReadBytePort(output, dut);
    testReadHalfPort(output, dut);
    testReadWordPort(output, dut);
    testReadQuadPort(output, dut);
    testReadArrayPort(output, dut);
    testReadQuadArrayPort(output, dut);
    testReadRegPort(output, dut);
    testReadWirePort(output, dut);
}
