#include "Signal.h"
#include <bitset>

using namespace SST::VerilatorSST;

Signal::Signal(uint16_t nBits) : nBits(nBits){
    uint16_t nBytes = calculateNumBytes(nBits);
    safe_size(nBytes);

    format = vpiStringVal;
    PLI_BYTE8 * buf = new PLI_BYTE8[nBytes+1];
    value.str = buf;
}

Signal::Signal(uint16_t nBits, uint64_t init_val) : Signal(nBits){
    uint64_t mask = nBits < 64 ? (1 << nBits) - 1 : UINT64_MAX - 1;
    uint64_t masked_val = init_val & mask;

    uint64_t nBytes = calculateNumBytes(nBits);
    for(uint64_t i = 0; i < nBytes; i++){
        uint8_t byte = (masked_val >> (i*8)) & 255;
        PLI_BYTE8 castSafeByte = static_cast<PLI_BYTE8>(byte);
        value.str[nBytes - i - 1] = castSafeByte;
    }
    value.str[nBytes] = '\0';
}

Signal::Signal(const Signal& other) : Signal(other.nBits){
    uint16_t nBytes = calculateNumBytes(other.nBits);
    std::copy(other.value.str,other.value.str + (nBytes+1),value.str);
}

void Signal::safe_size(uint16_t nBytes){
    assert(nBytes <= maxStrSize);
}

uint16_t Signal::getNumBits(){
    return nBits;
}

uint16_t Signal::calculateNumBytes(uint16_t nBits){
    const uint16_t ret = ((nBits/8) + (nBits % 8 != 0));
    return ret;
}

uint16_t Signal::getNumBytes(){
    const uint16_t ret = calculateNumBytes(nBits);
    return ret;
}
