#ifndef _SIGNAL_H_
#define _SIGNAL_H_

#include <type_traits>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <memory>
#include "vpi_user.h"
#include "verilatedos.h"

#define SIGNAL_VPI_FORMAT vpiVectorVal
#define SIGNAL_LOW (uint64_t) 0
#define SIGNAL_HIGH (uint64_t) 1
#define SIGNAL_BITS_MAX VL_VALUE_STRING_MAX_WORDS * VL_EDATASIZE * 8

namespace SST::VerilatorSST {
using signal_depth_t = uint64_t;
using signal_width_t = uint32_t;

class SignalFactory;

class Signal {
    friend class SignalFactory;

    private:
    //private members
    signal_width_t nBits;
    signal_depth_t depth;
    p_vpi_value storage;

    //construction validators
    void validate(signal_width_t nBits, signal_depth_t depth);

    //constructors and destructors
    Signal(signal_width_t nBits, signal_depth_t depth, p_vpi_value storage);

    public:
    Signal(const Signal& other);
    Signal(Signal&& other);
    Signal(signal_width_t nBits, uint64_t init_val);
    Signal(signal_width_t nBits, uint8_t * init_val);
    Signal(signal_width_t nBits, signal_depth_t depth, uint8_t * init_val, bool descending);
    ~Signal();
    Signal& operator=(Signal other);

    //private member accessors
    signal_width_t getNumBits() const;
    signal_depth_t getDepth() const;

    //storage representation accessors
    uint8_t getUIntScalar() const;
    uint8_t * getUIntVector(signal_depth_t depth) const;
    uint8_t * getUIntArray(bool reverse) const;
    s_vpi_value getVpiValue(signal_depth_t depth) const;


    //helper functions
    void swap(Signal& first, Signal& second);
    
    static signal_width_t calculateNumBytes(signal_width_t nBits);
    static signal_width_t calculateNumWords(signal_width_t nBits);
    static uint32_t * uint8ArrToUint32Arr(const uint8_t * const src, const signal_width_t size, const signal_depth_t rows);
    static uint8_t * uint32ArrToUint8Arr(const uint32_t * const src, const signal_width_t size, const signal_depth_t rows);
};

class SignalFactory {
    private: 
    signal_width_t bits = 0;
    signal_width_t words = 0;
    signal_depth_t depth = 0;
    signal_depth_t nextRow = 0;
    p_vpi_value storage;

    public:
    SignalFactory(signal_width_t nBits, signal_depth_t depth);
    SignalFactory();
    ~SignalFactory();

    Signal * operator()(const s_vpi_value &row);
};

}
#endif
