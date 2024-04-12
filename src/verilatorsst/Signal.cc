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
    assert(nBits < 64);
    uint64_t mask = (static_cast<uint64_t>(1) << 32) - 1;
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

template<typename T>
T Signal::getUIntScalarHelper(uint16_t nBytes, PLI_BYTE8 * storage){
    T ret = 0;
    auto sizeT = sizeof(T);
    for(uint16_t i = 0; i<nBytes; i++){
        PLI_BYTE8 byte = storage[i];
        uint8_t castSafeByte = static_cast<uint8_t>(byte);
        uint8_t padSafeByte = (castSafeByte == ' ') ? 0 : castSafeByte; //TODO https://github.com/verilator/verilator/issues/5036
        ret |= padSafeByte << ((sizeT-i-1)*8);
    }
    return ret;
}

template<typename T>
T Signal::getUIntScalar() {
    static_assert(std::is_unsigned_v<T> == true);
    const uint16_t nBytesStored = getNumBytes();
    assert(sizeof(T) >= nBytesStored);

    T ret = getUIntScalarHelper<T>(nBytesStored,value.str);
    return ret;
}

template<typename T>
T* Signal::getUIntVector(int wordSizeBits) {
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
