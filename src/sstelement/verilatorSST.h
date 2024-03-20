#ifndef _VERILATOR_SST_H_
#define _VERILATOR_SST_H_

#include <map>
#include <string>
#include <functional>
#include <variant>
#include <utility>
#include <memory>
#include "Top.h"

namespace SST::VerilatorSST {
using PortType = std::variant<CData*, SData*, IData*, QData* >;

class VerilatorSST {
    private:
    using SelfType = Top;
    using function_t = std::function<PortType(VerilatorSST&)>;
    using map_t = std::map<std::string, PortType>;
    std::unique_ptr<VerilatedContext> contextp;
    std::unique_ptr<SelfType> top;
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
    template<typename T>
    void writePort(std::string port, const T & data);
    template<typename T>
    void readPort(std::string port, T & data);
    // void tick(uint64_t add);
    void tick(uint64_t add, std::string port);
    uint64_t getCurrentTick();
    void finish();

};


}

#endif
