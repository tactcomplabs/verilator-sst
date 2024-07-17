#include "WritePortTestBench.h"

using namespace SST::VerilatorSST;

void testWritePort(SST::Output * output, SST::VerilatorSST::VerilatorSSTBase * dut){
    const std::vector<uint8_t> write_port_v = {1};
    dut->writePort("write_port", write_port_v);
    dut->clock(-1);
    const std::vector<uint8_t> read_write_port_v = dut->readPort("read_write_port");
    CHECK_EQ(read_write_port_v.size(),1);
    CHECK_EQ(read_write_port_v[0],read_write_port_v[0]);
}

void testWriteBytePort(SST::Output * output, SST::VerilatorSST::VerilatorSSTBase * dut){
    std::vector<uint8_t> write_byte_port_v;
    write_byte_port_v.resize(1);
    for(int i = 0; i < write_byte_port_v.size(); i++) {
        write_byte_port_v[i] = i;
    }
    dut->writePort("write_byte_port", write_byte_port_v);
    dut->clock(-1);
    const std::vector<uint8_t> read_write_byte_port_v = dut->readPort("read_write_byte_port");
    CHECK_EQ(read_write_byte_port_v.size(),write_byte_port_v.size());
    for(int i = 0; i < write_byte_port_v.size(); i++) {
        CHECK_EQ(read_write_byte_port_v[i],write_byte_port_v[i]);
    }
}

void testWriteHalfPort(SST::Output * output, SST::VerilatorSST::VerilatorSSTBase * dut){
    std::vector<uint8_t> write_half_port_v;
    write_half_port_v.resize(2);
    for(int i = 0; i < write_half_port_v.size(); i++) {
        write_half_port_v[i] = i;
    }
    dut->writePort("write_half_port", write_half_port_v);
    dut->clock(-1);
    const std::vector<uint8_t> read_write_half_port_v = dut->readPort("read_write_half_port");
    CHECK_EQ(read_write_half_port_v.size(),write_half_port_v.size());
    for(int i = 0; i < write_half_port_v.size(); i++) {
        CHECK_EQ(read_write_half_port_v[i],write_half_port_v[i]);
    }
}

void testWriteWordPort(SST::Output * output, SST::VerilatorSST::VerilatorSSTBase * dut){
    std::vector<uint8_t> write_word_port_v;
    write_word_port_v.resize(4);
    for(int i = 0; i < write_word_port_v.size(); i++) {
        write_word_port_v[i] = i;
    }
    dut->writePort("write_word_port", write_word_port_v);
    dut->clock(-1);
    const std::vector<uint8_t> read_write_word_port_v = dut->readPort("read_write_word_port");
    CHECK_EQ(read_write_word_port_v.size(),write_word_port_v.size());
    for(int i = 0; i < write_word_port_v.size(); i++) {
        CHECK_EQ(read_write_word_port_v[i],write_word_port_v[i]);
    }
}

void testWriteDoublePort(SST::Output * output, SST::VerilatorSST::VerilatorSSTBase * dut){
    std::vector<uint8_t> write_double_port_v;
    write_double_port_v.resize(8);
    for(int i = 0; i < write_double_port_v.size(); i++) {
        write_double_port_v[i] = i;
    }
    dut->writePort("write_double_port", write_double_port_v);
    dut->clock(-1);
    const std::vector<uint8_t> read_write_double_port_v = dut->readPort("read_write_double_port");
    CHECK_EQ(read_write_double_port_v.size(),write_double_port_v.size());
    for(int i = 0; i < write_double_port_v.size(); i++) {
        CHECK_EQ(read_write_double_port_v[i],write_double_port_v[i]);
    }
}

void testWriteQuadPort(SST::Output * output, SST::VerilatorSST::VerilatorSSTBase * dut){
    // SKIP_TEST; //TODO
    std::vector<uint8_t> write_quad_port_v;
    write_quad_port_v.resize(16);
    for(int i = 0; i < write_quad_port_v.size(); i++) {
        write_quad_port_v[i] = i;
    }
    dut->writePort("write_quad_port", write_quad_port_v);
    dut->clock(-1);
    const std::vector<uint8_t> read_write_quad_port_v = dut->readPort("read_write_quad_port");
    CHECK_EQ(read_write_quad_port_v.size(),write_quad_port_v.size());
    for(int i = 0; i < write_quad_port_v.size(); i++) {
        CHECK_EQ(read_write_quad_port_v[i],write_quad_port_v[i]);
    }
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
    testWriteBytePort(output, dut);
    testWriteHalfPort(output, dut);
    testWriteWordPort(output, dut);
    testWriteDoublePort(output, dut);
    testWriteQuadPort(output,dut);
    testWriteArrayPort(output, dut);
    testWriteQuadArrayPort(output, dut);
    testWriteRegPort(output, dut);
    testWriteWirePort(output, dut);
}
