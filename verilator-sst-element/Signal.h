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

class SignalFactory;

class Signal {
  friend class SignalFactory;

  private:
  //private members
    uint32_t nBits;
    uint64_t depth;
    p_vpi_value storage;

    //construction validators
    void validate(uint32_t nBits, uint64_t depth);

    //constructors and destructors
    Signal(uint32_t nBits, uint64_t depth, p_vpi_value storage);

  public:
    Signal(const Signal& other);
    //Signal(Signal&& other);
    //Signal(uint32_t nBits, uint64_t init_val);
    Signal(uint32_t nBits, std::vector<uint8_t> init_val);
    Signal(uint32_t nBits, uint64_t depth, std::vector<uint8_t> init_val, bool descending);
    ~Signal();
    Signal& operator=(Signal other);

    //private member accessors
    uint32_t getNumBits() const;
    uint64_t getDepth() const;

    //storage representation accessors
    uint8_t getUIntScalar() const;
    std::vector<uint8_t> getUIntArray(uint64_t depth) const;
    std::vector<uint8_t> getUIntVector(bool reverse) const;
    s_vpi_value getVpiValue(uint64_t depth) const;


    //helper functions
    void swap(Signal& first, Signal& second);
    
    static uint32_t calculateNumBytes(uint32_t nBits);
    static uint32_t calculateNumWords(uint32_t nBits);
    static std::vector<uint32_t> uint8ArrToUint32Arr(const std::vector<uint8_t> src, const uint32_t size, const uint64_t rows);
    static std::vector<uint8_t> uint32ArrToUint8Arr(const std::vector<uint32_t> src, const uint32_t size, const uint64_t rows);
};

class SignalFactory {
  private: 
    uint32_t bits = 0;
    uint32_t words = 0;
    uint64_t depth = 0;
    uint64_t nextRow = 0;
    p_vpi_value storage;

  public:
    SignalFactory(uint32_t nBits, uint64_t depth);
    SignalFactory();
    ~SignalFactory();

    Signal * operator()(const s_vpi_value &row);
};

}
#endif // _SIGNAL_H_
