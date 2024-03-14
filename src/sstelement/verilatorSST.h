#ifndef _VERILATOR_SST_H_
#define _VERILATOR_SST_H_

#include <map>
#include <string>
#include <functional>
#include <variant>
#include <utility>
#include "macros.h"
#include "verilated.h"
#include "Top.h"
#include <sst/core/component.h>

namespace SST {

class VerilatorSST : private Top {
    private:
    using SelfType = Top;
    using PortType = std::variant< std::monostate, CData*, SData*, IData*, QData*, EData*, WData*, WDataInP*, WDataOutP* >;
    PortType get_clk() { return PortType{&clk};} 
    PortType get_reset_l() { return PortType{&reset_l};} 
    PortType get_done() { return PortType{&done};} 
    PortType get_stop() { return PortType{&stop};} 
    using function_t = std::function<PortType(VerilatorSST&)>;
    using map_t = std::map<std::string, function_t>;
    map_t init_reflect_values() { 
        map_t ret{};
        std::vector< std::pair<std::string, function_t> > values = { 
            std::make_pair<std::string, function_t>(std::string{"clk"}, [](VerilatorSST & self) { return self.get_clk();}), 
            std::make_pair<std::string, function_t>(std::string{"reset_l"}, [](VerilatorSST & self) { return self.get_reset_l();}),
            std::make_pair<std::string, function_t>(std::string{"done"}, [](VerilatorSST & self) { return self.get_done();}),
            std::make_pair<std::string, function_t>(std::string{"stop"}, [](VerilatorSST & self) { return self.get_stop();}),
        };
        for(auto v : values) { ret.insert(v);} return ret;
    } 

    public:
    VerilatorSST();
    virtual ~VerilatorSST();
    void writeInputPort(std::string port, PortType value);
    void readOutputPort(std::string port, PortType value);
    map_t reflect_values = init_reflect_values();
};
}
#endif


