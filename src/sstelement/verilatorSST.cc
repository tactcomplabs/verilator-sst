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
    reflect_values = init_reflect_values();
    std::cout << "VerilatorSST() finish" << std::endl;
}
VerilatorSST::~VerilatorSST() {
}

template<typename T>
void VerilatorSST::writePort(std::string portName, const T & data){
    std::cout << "writePort(std::string, const T&) start" << std::endl;
    auto search = reflect_values.find(portName);
    if (search == reflect_values.end()){
        throw std::runtime_error("Port not found");
    }

    PortType port = search->second;
    std::cout << "writePort(std::string, const T&) found port" << std::endl;

    if(!std::holds_alternative<T*>(port)){
        throw std::runtime_error("VerilatorSST::writePort(std::string, const T&): Incoming data does not match port type");
    }
    std::cout << "writePort(std::string, const T&) type match" << std::endl;

    std::cout << "writing " << +data << " to port " << search->first << std::endl;

    *(std::get<T*>(port)) = data;
    
    std::cout << "clk=" << +(top->clk) << std::endl;
    std::cout << "reset_l=" << +(top->reset_l) << std::endl;
    std::cout << "stop=" << +(top->stop) << std::endl;

    top->eval();
    std::cout << "writePort(std::string, const T&) finish" << std::endl;
}

template<typename T>
void VerilatorSST::readPort(std::string portName, T & data) {
    std::cout << "readPort(std::string, T&) start" << std::endl;
    auto search = reflect_values.find(portName);
    if (search == reflect_values.end()){
        throw std::runtime_error("Port not found");
    }

    PortType port = search->second;

    if(!std::holds_alternative<T*>(port)){
        throw std::runtime_error("VerilatorSST::readPort(std::string, T&): Incoming data does not match port type");
    }

    std::cout << "reading ";
    std::visit([](auto d){std::cout << +*d;}, port);
    std::cout << " from port " << search->first << std::endl;

    data = *(std::get<T*>(port));
    
    std::cout << "readPort(std::string, T&) finish" << std::endl;
}

void VerilatorSST::tick(uint64_t t, std::string clockPort){
    std::cout << "tick() start" << std::endl;
    uint8_t clk;
    readPort(clockPort, clk);
    clk = ~clk;
    writePort(clockPort, clk);

    contextp->timeInc(t);
    std::cout << "tick() finish" << std::endl;
}

uint64_t VerilatorSST::getCurrentTick(){
    return contextp->time();
}

void VerilatorSST::finish(){
    top->final();
    finalCallback();
}
