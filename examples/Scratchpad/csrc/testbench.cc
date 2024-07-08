// #include "Vtop.h"
#include "verilated.h"
#include "vpi_user.h"
#include "verilated_vpi.h"
#include "../obj_dir/VTop.h"
#include <memory>

#define CHUNK_SIZE 10
#define NUM_CHUNKS 10
#define SCRATCHPAD_BASE 16
#define SCRATCHPAD_SIZE (CHUNK_SIZE * NUM_CHUNKS)

#define BYTE 0
#define HALF 1
#define WORD 2
#define DOUBLE 3

void clock(VTop* top) {
    top->clk = ~(top->clk);
    top->eval();
    top->clk = ~(top->clk);
    top->eval();
}

void writeOp(VTop * top, int addr, int len, uint64_t wdata) {
    top->en = 1;
    top->write = 1;
    top->addr = SCRATCHPAD_BASE + addr;
    top->len = len;
    top->wdata = wdata;
    clock(top);
    top->en = 0;
    printf("testbench:wdata=%llx addr=%u\n",wdata,addr);
}

uint64_t readOp(VTop * top, int addr, int len) {
    top->en = 1;
    top->write = 0;
    top->addr = SCRATCHPAD_BASE + addr;
    top->len = len;
    clock(top);
    top->en = 0;
    uint64_t rdata = top->rdata;
    printf("testbench:rdata=%llx addr=%u\n",rdata,addr);
    return rdata;
}

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    const std::unique_ptr<VerilatedContext> contextp{new VerilatedContext};
    VTop* top{new VTop{contextp.get()}};

    //set reset vals
    top->rst_l = 0;
    top->en = 0;
    top->write = 0;
    top->addr = 0;
    top->len = 0;
    top->wdata = 0;
    clock(top);

    //release reset
    top->rst_l = 1;
    clock(top);
    
    //write read byte
    printf("\nBYTE TEST...\n");
    writeOp(top,0,BYTE,0xAB);
    assert(readOp(top,0,BYTE) == 0xAB);
    
    //write read half
    printf("\nHALF TEST...\n");
    writeOp(top,0,HALF,0xBEEF);
    assert(readOp(top,0,HALF) == 0xBEEF);

    //write read word
    printf("\nWORD TEST...\n");
    writeOp(top,0,WORD,0xCAFEF00D);
    assert(readOp(top,0,WORD) == 0xCAFEF00D);

    //write read double
    printf("\nDOUBLE TEST...\n");
    uint64_t testDouble = (static_cast<uint64_t>(0xDEADC0DE) << 32) + 0xBAADBABE;
    writeOp(top,0,DOUBLE, testDouble);
    assert(readOp(top,0,DOUBLE) == testDouble);

    //random writes
    printf("\nRANDOM TEST...\n");
    uint8_t data_byte[SCRATCHPAD_SIZE];
    int addrs[100];
    for(auto i = 0; i<100; i++) {
        int addr = rand() % SCRATCHPAD_SIZE;
        addrs[i] = addr;
        data_byte[addr] = static_cast<uint8_t>(rand());
        writeOp(top,addr,BYTE,data_byte[addr]);
    }

    for(auto i = 0; i<100; i++) {
        int addr = addrs[i];
        assert(static_cast<uint8_t>(readOp(top,addr,BYTE)) == data_byte[addr]); 
    }

    uint64_t data_double[SCRATCHPAD_SIZE/8];
    for(auto i = 0; i<100; i++) {
        int addr = (rand() % (SCRATCHPAD_SIZE/8 - 1)) * 8;
        addrs[i] = addr;
        data_double[addr/8] = (static_cast<uint64_t>(rand()) << 32) | rand();
        writeOp(top,addr,DOUBLE,data_double[addr/8]);
    }

    for(auto i = 0; i<100; i++) {
        int addr = addrs[i];
        assert(readOp(top,addr,DOUBLE) == data_double[addr/8]); 
    }

    delete top;

    printf("ALL TESTS PASSED!\n");
}
