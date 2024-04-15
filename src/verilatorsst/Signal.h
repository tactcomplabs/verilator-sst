#ifndef _SIGNAL_H_
#define _SIGNAL_H_

#include <type_traits>
#include <algorithm>
#include <cassert>
#include "vpi_user.h"
#include "verilatedos.h"

namespace SST::VerilatorSST {
const int SIGNAL_BITS_MAX = VL_VALUE_STRING_MAX_WORDS * VL_EDATASIZE * 8;
using signal_depth_t = uint64_t;
using signal_width_t = uint32_t;

class Signal {
    private:
    signal_width_t nBits;
    signal_depth_t depth;
    PLI_BYTE8 * storage;

    template<typename T>
    T getUIntScalarInternal(signal_width_t nBytes, signal_depth_t offset);

    public: 
    Signal() : Signal(1){};
    Signal(const Signal& other);
    Signal(signal_width_t nBits);
    Signal(signal_width_t nBits, uint64_t init_val);
    Signal(signal_width_t nBits, signal_depth_t depth, PLI_BYTE8 * init_val);
    Signal(signal_width_t nBits, signal_depth_t depth, uint64_t * init_val);
    ~Signal(){ delete storage; };

    signal_width_t getNumBits();
    signal_width_t getNumBytes();
    signal_depth_t getDepth();

    template<typename T>
    T getUIntScalar();
    template<typename T>
    T* getUIntVector();

    t_vpi_value getVpiValue(signal_depth_t depth);

    static signal_width_t calculateNumBytes(signal_width_t nBits);
};
}
#include "Signal.tcc"
#endif
