#ifndef _VERILATOR_SST_H_
#define _VERILATOR_SST_H_

#include <string>
#include <functional>
#include <stdexcept>
#include "VTop.h"
#include "verilated_vpi.h"
#include "Signal.h"

namespace SST::VerilatorSST {

class VerilatorSST {
    private:
    std::unique_ptr<VerilatedContext> contextp;
    std::unique_ptr<VTop> top;

    public:
    VerilatorSST();
    ~VerilatorSST(){};
    void writePort(std::string portName, Signal & val);
    void readPort(std::string portName, Signal & val);
    void clockTick(uint64_t add, std::string port);
    uint64_t getCurrentTick();
    void finish();

};
}
#endif
