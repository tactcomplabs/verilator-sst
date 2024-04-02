#ifndef _SIGNAL_VALUE_H_
#define _SIGNAL_VALUE_H_

#include <stdexcept>
#include "vpi_user.h"
#include "verilatedos.h"

namespace SST::VerilatorSST {
const int maxStrSize = VL_VALUE_STRING_MAX_WORDS * VL_EDATASIZE;

class Signal : public t_vpi_value {
    private:
        const uint16_t nBits;
        void safe_size(uint16_t nBits);
        uint16_t calculateNumBytes(uint16_t nBits);

    public:
    Signal() : Signal(1){};
    Signal(uint16_t nBits);
    Signal(uint16_t nBits, uint64_t init_val);
    ~Signal(){};
    
    uint16_t getNumBits();
    uint16_t getNumBytes();

    template<typename T>
    T getUIntValue() {
        static_assert(std::is_unsigned_v<T> == true);
        const uint16_t nBytesReq = sizeof(T);
        const uint16_t nBytesStored = getNumBytes();

        uint8_t ret_arr[nBytesReq];
        auto nBytesDiff = (nBytesReq-nBytesStored);

        if(nBytesDiff < 0){
            std::runtime_error("Signal::getUIntValue(): signal size is greater than the requested uint type");
        }

        int i;
        for(i = 0; i<nBytesDiff; i++){
            ret_arr[i] = 0;
        }

        for(int j = i; j<nBytesReq; j++){
            uint8_t byte = value.str[j-i];
            ret_arr[j] = (byte == ' ') ? 0 : byte;
        }

        T ret = *(T*)ret_arr;
        return ret;
    }
};
}
#endif
