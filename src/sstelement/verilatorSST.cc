#include "Top.h"
#include "verilatorSST.h"
// #include "macros.h"
// #include <map>
// #include <string>
// #include <functional>
// #include <variant>
// #include <utility>

SST::VerilatorSST::VerilatorSST() : Top{"top"} {}

SST::VerilatorSST::~VerilatorSST() {}

// // template <typename P>
// // void VerilatorSST::writeInputPort(P* port, P* data){
// //     // static_assert(std::is_unsigned<P>::value);
// //     // *port = *data;
// //     // this->eval();
// // }

// // template <typename P>
// // void VerilatorSST::readOutputPort(P* port, P* data){
// //     // static_assert(std::is_unsigned<P>::value);
// //     // using map_t = std::map<std::string, function_t>;
// //     // map_t reflect_value{};
// //     // *data = *port;
// // }

// //REFLECT_INIT(Test);

// // #include <iostream>

// // int main() {
// //    Test t{};
// //    for(auto i : Test::reflect_values) {
// //       std::cout << i.first << std::endl;
// //       Test::PortType v = i.second(t);
// //       if(v.index() == 1) {
// //          std::cout << std::get<1>(v) << std::endl;
// //       }
// //       else if(std::holds_alternative<SData*>(v)) {
// //          std::cout << std::get<SData*>(v) << std::endl;
// //       }
// //    }
// // }