#ifndef _SIGNAL_VALUE_H_
#define _SIGNAL_VALUE_H_

#include <algorithm>
#include <cassert>
#include "vpi_user.h"
#include "verilatedos.h"

namespace SST::VerilatorSST {
const int maxStrSize = VL_VALUE_STRING_MAX_WORDS * VL_EDATASIZE;

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

    template<typename T>
    T getUIntValue() {
        static_assert(std::is_unsigned_v<T> == true);
        const uint16_t nBytesStored = getNumBytes();

        assert(sizeof(T) >= nBytesStored);

        T ret = 0;
        for(uint16_t i = 0; i<nBytesStored; i++){
            PLI_BYTE8 byte = value.str[nBytesStored-1-i];
            uint8_t castSafeByte = static_cast<uint8_t>(byte);
            uint8_t padSafeByte = (castSafeByte == ' ') ? 0 : castSafeByte; //TODO https://github.com/verilator/verilator/issues/5036
            ret |= castSafeByte << (i*8);
        }

        return ret;
    }
};
}
#endif
