#ifndef _VERILATOR_SST_H_
#define _VERILATOR_SST_H_

#include <string>
#include <cassert>

#include "verilated_vpi.h"
#include "verilated.h"
#include "Signal.h"

namespace SST::VerilatorSST {
struct SignalQueueEntry {
    std::string portName;
    uint64_t writeTick;
    Signal signal;
};

template<typename T>
class VerilatorSST {
    private:
    std::unique_ptr<VerilatedContext> contextp;
    std::unique_ptr<T> top;
    std::unique_ptr<std::vector<SignalQueueEntry>> signalQueue;
    
    void pollSignalQueue();
    uint8_t maskShiftL(uint8_t data, uint8_t mask, int shift);
    void writeHelper(uint8_t word, uint16_t wordSizeBits, int bitStart, PLI_BYTE8 * storage);
    bool isFinished = false;

    public:
    VerilatorSST();
    ~VerilatorSST(){};
    
    void writePort(std::string portName, Signal & val);
    void writePortAtTick(std::string portName, Signal & signal, uint64_t tick);
    void readPort(std::string portName, Signal & val);

    void tick(uint64_t elapse);
    void tickClockPeriod(std::string clockPort);
    uint64_t getCurrentTick();

    void finish();
};
}
#endif
