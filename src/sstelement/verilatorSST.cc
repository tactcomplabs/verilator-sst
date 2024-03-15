#include "verilatorSST.h"
#include <iostream>
// #include "macros.h"
// #include <map>
// #include <string>
// #include <functional>
// #include <variant>
// #include <utility>

using namespace SST::VerilatorSST;

// VerilatorSST::VerilatorSST() : a(1), b(2), c(3) {};

VerilatorSST::VerilatorSST() {
    top = std::make_unique<Top>("top");
}
VerilatorSST::~VerilatorSST() {}

void VerilatorSST::writeInputPort(std::string port, PortType data){
    // auto p = reflect_values.find(port);
    // uint8_t portTypeIndex = data.index();
    // if(portTypeIndex != p->second(this)->index()){
    //     return;
    // }
    // std::cout << "writing " << std::get<data.index>(data) << " to port " << p->first << std::endl;
    // *(p->second(this)) = std::get<data.index>(data);
}

void readOutputPort(std::string port, PortType data) {
    // VerilatorSST::map_t::iterator p = reflect_values.find(port);
    // std::cout << "reading " << p->second << " from port " << p->first << std::endl;
    // *data = p->second;
}

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