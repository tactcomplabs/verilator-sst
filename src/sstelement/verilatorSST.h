#ifndef _VERILATOR_SST_H_
#define _VERILATOR_SST_H_

#include <map>
#include <string>
#include <functional>
#include <variant>
#include <utility>
#include <memory>
#include <iostream>
#include <stdexcept>
#include "Top.h"

namespace SST::VerilatorSST {

class VerilatorSST {
    private:
    using PortType = Top::PortType;
    using function_t = std::function<PortType(VerilatorSST&)>;
    using map_t = std::map<std::string, PortType>;
    std::unique_ptr<VerilatedContext> contextp;
    std::unique_ptr<Top> top;

    std::function<void()> finalCallback;
    public:
    VerilatorSST(std::function<void()>);
    ~VerilatorSST();


    template<typename T>
    void writePort(std::string portName, const T & data){
        std::cout << "writePort(std::string, const T&) start" << std::endl;
        auto search = top->reflect_values.find(portName);
        if (search == top->reflect_values.end()){
            throw std::runtime_error("Port not found");
        }

        PortType port = search->second.second;
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
    void readPort(std::string portName, T & data) {
    std::cout << "readPort(std::string, T&) start" << std::endl;
    auto search = top->reflect_values.find(portName);
    if (search == top->reflect_values.end()){
        throw std::runtime_error("Port not found");
    }

    PortType port = search->second.second;

    if(!std::holds_alternative<T*>(port)){
        throw std::runtime_error("VerilatorSST::readPort(std::string, T&): Incoming data does not match port type");
    }

    std::cout << "reading ";
    std::visit([](auto d){std::cout << +*d;}, port);
    std::cout << " from port " << search->first << std::endl;

    data = *(std::get<T*>(port));
    
    std::cout << "readPort(std::string, T&) finish" << std::endl;
}
    // void tick(uint64_t add);
    void clockTick(uint64_t add, std::string port);
    uint64_t getCurrentTick();
    void finish();
    void getAllPortNames(std::vector<std::string> & ports);
};


}

#endif
