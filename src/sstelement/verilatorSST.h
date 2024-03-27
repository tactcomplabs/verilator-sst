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
#include "VTop.h"
#include "verilated_vpi.h"

namespace SST::VerilatorSST {
using PortType = std::variant<CData*, SData*, IData*, QData* >;

class VerilatorSST {
    private:
    using function_t = std::function<PortType(VerilatorSST&)>;
    using map_t = std::map<std::string, PortType>;
    std::unique_ptr<VerilatedContext> contextp;
    std::unique_ptr<VTop> top;
    map_t reflect_values;
    map_t init_reflect_values() { 
        map_t ret{};
        std::vector< std::pair<std::string, PortType> > values = { 
            std::make_pair<std::string, PortType>(std::string{"clk"}, PortType{&(top->clk)}),
            std::make_pair<std::string, PortType>(std::string{"reset_l"}, PortType{&(top->reset_l)}),
            std::make_pair<std::string, PortType>(std::string{"done"}, PortType{&(top->done)}),
            std::make_pair<std::string, PortType>(std::string{"stop"}, PortType{&(top->stop)}),
        };
        for(auto v : values) { ret.insert(v);} return ret;
    }

    std::function<void()> finalCallback;
    public:
    VerilatorSST(std::function<void()>);
    ~VerilatorSST();


    void writePort(std::string portName, const PLI_BYTE8 data){
        std::cout << "writePort(std::string, const T&) start" << std::endl;
        char * name;
        strcpy(name,portName.c_str());
        vpiHandle vh1 = vpi_handle_by_name(name, NULL);
        if (!vh1){
            throw std::runtime_error("Port not found");
        }

        std::cout << "writing " << +data << " to port " << portName << std::endl;

        s_vpi_time vpi_time_s;
        vpi_time_s.type = vpiSimTime;
        vpi_time_s.high = 0;
        vpi_time_s.low = 0;
        s_vpi_value v;
        v.value.integer = static_cast<PLI_INT32>(data);
        v.format = vpiIntVal;
        vpi_put_value(vh1,&v,&vpi_time_s,vpiInertialDelay);

        std::cout << "clk=" << +(top->clk) << std::endl;
        std::cout << "reset_l=" << +(top->reset_l) << std::endl;
        std::cout << "stop=" << +(top->stop) << std::endl;

        top->eval();
        std::cout << "writePort(std::string, const T&) finish" << std::endl;
    }   


    template<typename T>
    void readPort(std::string portName, T & data) {
        std::cout << "readPort(std::string, T&) start" << std::endl;

        char * name;
        strcpy(name,portName.c_str());

        vpiHandle vh1 = vpi_handle_by_name(name, NULL);;
        if (!vh1){
            throw std::runtime_error("Port not found");
        }

        s_vpi_value v;
        v.format = vpiIntVal;
        vpi_put_value(vh1,&v,NULL,vpiNoDelay);

        const char* namee = vpi_get_str(vpiName, vh1);
        std::cout << "reading " << +v.value.integer<< " from port " << portName << std::endl;
        
        data = v.value.integer;

        std::cout << "readPort(std::string, T&) finish" << std::endl;
    }
    // void tick(uint64_t add);
    void clockTick(uint64_t add, std::string port);
    uint64_t getCurrentTick();
    void finish();

};


}

#endif
