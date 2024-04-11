#ifndef _SIGNAL_VALUE_H_
#define _SIGNAL_VALUE_H_

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
    T getUIntScalar() {
        static_assert(std::is_unsigned_v<T> == true);
        const uint16_t nBytesStored = getNumBytes();

        assert(sizeof(T) >= nBytesStored);

        T ret = 0;
        for(uint16_t i = 0; i<nBytesStored; i++){
            PLI_BYTE8 byte = value.str[i];
            uint8_t castSafeByte = static_cast<uint8_t>(byte);
            uint8_t padSafeByte = (castSafeByte == ' ') ? 0 : castSafeByte; //TODO https://github.com/verilator/verilator/issues/5036
            ret |= padSafeByte << (i*8);
        }
        return ret;
    }

    template<typename T>
    T* getUIntVector(int wordSizeBits) {
        static_assert(std::is_unsigned_v<T> == true);
        assert(wordSizeBits != 0);
        assert(sizeof(T)*8 >= wordSizeBits);
        assert((nBits % wordSizeBits) == 0);

        auto vectorSize = nBits / wordSizeBits;
        assert(vectorSize > 0);
        
        T* ret = new T[vectorSize]();

        int i= 0;
        int bitStart = 0;
        for(uint16_t i = 0; i<vectorSize; i++){
            auto bitStart = i*wordSizeBits;
            auto byteArrayIdx = bitStart / 8;
            auto localBitStart = bitStart - byteArrayIdx*8;
            auto shift = 8-localBitStart-wordSizeBits;
            assert(shift >= 0 && "word size is not a factor of 8");
            uint8_t mask = (((1<<wordSizeBits)-1) << (8-wordSizeBits)) >> localBitStart;

            PLI_BYTE8 byte = value.str[byteArrayIdx];
            uint8_t castSafeByte = static_cast<uint8_t>(byte);
            uint8_t padSafeByte = (castSafeByte == ' ') ? 0 : castSafeByte;

            uint8_t wordMasked = padSafeByte & mask;
            uint8_t wordAligned = wordMasked >> shift;
            ret[i] = static_cast<T>(wordAligned);
        }
        return ret;
    }
};
}
#endif
