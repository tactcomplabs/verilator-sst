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

class Signal : public t_vpi_value {
    private:
    uint16_t nBits;
    void safe_size(uint16_t nBits);
    uint16_t calculateNumBytes(uint16_t nBits);

    template<typename T>
    T getUIntScalarHelper(uint16_t nBytes, PLI_BYTE8 * storage);
    public:
    Signal() : Signal(1){};
    Signal(const Signal& other);
    Signal(uint16_t nBits);
    Signal(uint16_t nBits, uint64_t init_val);
    ~Signal(){};

    uint16_t getNumBits();
    uint16_t getNumBytes();
    bool getSingleBit();

    template<typename T>
    T getUIntScalar();
    template<typename T>
    T* getUIntVector(int wordSizeBits);
};
}
#include "Signal.cc"
#endif
