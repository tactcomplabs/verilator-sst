#include "Top.h"
#include "verilatorSST.h"
#include "macros.h"
#include <map>
#include <string>
#include <functional>
#include <variant>
#include <utility>

VerilatorSST::VerilatorSST() : Top{"top"} {}

VerilatorSST::~VerilatorSST() {}

// template <typename P>
// void VerilatorSST::writeInputPort(P* port, P* data){
//     // static_assert(std::is_unsigned<P>::value);
//     // *port = *data;
//     // this->eval();
// }

// template <typename P>
// void VerilatorSST::readOutputPort(P* port, P* data){
//     // static_assert(std::is_unsigned<P>::value);
//     // using map_t = std::map<std::string, function_t>;
//     // map_t reflect_value{};
//     // *data = *port;
// }

//REFLECT_INIT(Test);

// #include <iostream>

// int main() {
//    Test t{};
//    for(auto i : Test::reflect_values) {
//       std::cout << i.first << std::endl;
//       Test::PortType v = i.second(t);
//       if(v.index() == 1) {
//          std::cout << std::get<1>(v) << std::endl;
//       }
//       else if(std::holds_alternative<SData*>(v)) {
//          std::cout << std::get<SData*>(v) << std::endl;
//       }
//    }
// }

// template <class T>
// void VerilatedSST::setup(T* ret){
//     static_assert(std::is_base_of<VerilatedModel,T>::value, "T must inherit from VerilatedModel");
//     const contextp = new VerilatedContext;
//     contextp->debug(0);
//     contextp->randReset(2);
//     contextp->traceEverOn(true);
//     char ** empty = {};
//     contextp->commandArgs(0, empty);
//     ret = new T(contextp,"top");
// }