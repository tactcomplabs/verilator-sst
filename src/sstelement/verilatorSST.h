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

// class VerilatorSST : private Top {
//     private:
//         VerilatorSST();
//         virtual ~VerilatorSST();
//     public:
//         REFLECT(Top,clk,reset_l,done,stop);
//         template <typename P>
//         void writeInputPort(P* port, P value);
//         template <typename P>
//         void readOutputPort(P* port, P* value);
// };

struct VerilatorSST : private Top {
    VerilatorSST();
    virtual ~VerilatorSST();

    template <typename P>
    void writeInputPort(std::string port, P* value);
    template <typename P>
    void readOutputPort(std::string port, P* value);

    // REFLECT(Top,clk,reset_l,done,stop);

    using SelfType = Top;
    using PortType = std::variant< std::monostate, CData*, SData*, IData*, QData*, EData*, WData*, WDataInP*, WDataOutP* >;
    PortType get_clk() { return PortType{&clk};} 
    PortType get_reset_l() { return PortType{&reset_l};} 
    PortType get_done() { return PortType{&done};} 
    PortType get_stop() { return PortType{&stop};} 
    using function_t = std::function<PortType(VerilatorSST&)>;
    using map_t = std::map<std::string, function_t>;
    static map_t init_reflect_values() { 
        map_t ret{};
        std::vector< std::pair<std::string, function_t> > values = { 
            std::make_pair<std::string, function_t>(std::string{"clk"}, [](VerilatorSST & self) { return self.get_clk();}), 
            std::make_pair<std::string, function_t>(std::string{"reset_l"}, [](VerilatorSST & self) { return self.get_reset_l();}),
            std::make_pair<std::string, function_t>(std::string{"done"}, [](VerilatorSST & self) { return self.get_done();}),
            std::make_pair<std::string, function_t>(std::string{"stop"}, [](VerilatorSST & self) { return self.get_stop();}),
        };
        for(auto v : values) { ret.insert(v);} return ret;
    } 
    static inline map_t reflect_values = init_reflect_values();
};

#endif


