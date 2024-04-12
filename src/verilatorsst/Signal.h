#ifndef _SIGNAL_H_
#define _SIGNAL_H_

#include <type_traits>
#include <algorithm>
#include <cassert>
#include "vpi_user.h"
#include "verilatedos.h"
#include <iostream>

namespace SST::VerilatorSST {
const int maxStrSize = VL_VALUE_STRING_MAX_WORDS * VL_EDATASIZE;
const int maxSignalSize = maxStrSize*8;
const uint64_t maxSignalInit = UINT64_MAX;

class Signal {
    private:
    uint16_t nBits;

    public:
    PLI_BYTE8 * storage;//todo move to private after debug
    Signal() : Signal(1){};
    Signal(const Signal& other);
    Signal(uint16_t nBits);
    Signal(uint16_t nBits, uint64_t init_val);
    Signal(uint16_t nBits, PLI_BYTE8 * init_val);
    ~Signal(){ delete storage; };

    uint16_t getNumBits();
    uint16_t getNumBytes();
    bool getSingleBit();

    template<typename T>
    T getUIntScalar();
    template<typename T>
    T* getUIntVector(int wordSizeBits);

    t_vpi_value getVpiValue();
};
}
#include "Signal.cc"
#endif
