#include "verilatorSST.h"
#include <iostream>
#include <stdexcept>

using namespace SST::VerilatorSST;

VerilatorSST::VerilatorSST(std::function<void()> finalCallback) : 
    finalCallback(finalCallback)  {
    std::cout << "VerilatorSST() start" << std::endl;
    contextp = std::make_unique<VerilatedContext>();
    std::cout << "VerilatorSST() contextp init" << std::endl;
    contextp->debug(0);
    contextp->randReset(2);
    contextp->traceEverOn(true);
    const char* empty {};
    contextp->commandArgs(0, &empty);

    top = std::make_unique<Top>(contextp.get(), "TOP");
    std::cout << "VerilatorSST() finish" << std::endl;
}

VerilatorSST::~VerilatorSST() {
}

void VerilatorSST::clockTick(uint64_t t, std::string clockPort){
    uint8_t clk;
    readPort(clockPort, clk);
    clk = ~clk;
    writePort(clockPort, clk);

    contextp->timeInc(t);
}

uint64_t VerilatorSST::getCurrentTick(){
    return contextp->time();
}

void VerilatorSST::finish(){
    top->final();
    finalCallback();
}

void VerilatorSST::getAllPortNames(std::vector<std::string> & ports){
    ports.clear();
    for(auto it = top->reflect_values.begin(); it != top->reflect_values.end(); ++it){
        ports.push_back(it->first);
    }
}