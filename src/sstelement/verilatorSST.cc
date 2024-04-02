#include "verilatorSST.h"
#include <iostream>
#include <stdexcept>

using namespace SST::VerilatorSST;

VerilatorSST::VerilatorSST(std::function<void()> finalCallback) : 
    finalCallback(finalCallback)  {
    std::cout << "VerilatorSST() start" << std::endl;
    contextp = std::make_unique<VerilatedContext>();
    std::cout << "VerilatorSST() contextp init" << std::endl;
    contextp->debug(1);
    contextp->randReset(2);
    contextp->traceEverOn(true);
    const char* empty {};
    contextp->commandArgs(0, &empty);

    top = std::make_unique<VTop>(contextp.get(), "");
    contextp->internalsDump();
    std::cout << "VerilatorSST() finish" << std::endl;
}

VerilatorSST::~VerilatorSST() {
}

void VerilatorSST::readPort(std::string portName, Signal & val) {
    char *name = new char[portName.length() + 1];
    strcpy(name,portName.c_str());

    vpiHandle vh1 = vpi_handle_by_name(name, NULL);;
    if (!vh1){
        throw std::runtime_error("Port not found");
    }

    vpi_get_value(vh1, &val);
}

void VerilatorSST::writePort(std::string portName, Signal & val){
    char *name = new char[portName.length() + 1];
    strcpy(name, portName.c_str());
    
    vpiHandle vh1 = vpi_handle_by_name(name, NULL);
    if (!vh1){
        throw std::runtime_error("Port not found");
    }

    s_vpi_time vpi_time_s;
    vpi_time_s.type = vpiSimTime;
    vpi_time_s.high = 0;
    vpi_time_s.low = 0;
    vpi_put_value(vh1,&val,&vpi_time_s,vpiInertialDelay);

    top->eval();
}

void VerilatorSST::clockTick(uint64_t t, std::string clockPort){
    Signal clk;
    readPort(clockPort, clk);

    uint8_t not_clk = ~clk.getUIntValue<uint8_t>();
    uint8_t next_clk = not_clk & 1;
    Signal nxt_clk = Signal(1, next_clk);
    writePort(clockPort, nxt_clk);

    contextp->timeInc(t);
}

uint64_t VerilatorSST::getCurrentTick(){
    return contextp->time();
}

void VerilatorSST::finish(){
    top->final();
    finalCallback();
}
