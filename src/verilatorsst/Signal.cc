#include "Signal.h"
#include <bitset>

using namespace SST::VerilatorSST;

uint16_t calculateNumBytes(uint16_t nBits){
    const uint16_t ret = ((nBits/8) + (nBits % 8 != 0));
    return ret;
}

Signal::Signal(uint16_t nBits) : nBits(nBits){
    uint16_t nBytes = calculateNumBytes(nBits);
    assert(nBytes <= maxStrSize);

    storage = new PLI_BYTE8[nBytes+1];
}

Signal::Signal(uint16_t nBits, uint64_t init_val) : Signal(nBits){
    assert(nBits < 64);
    uint64_t mask = (static_cast<uint64_t>(1) << 32) - 1;
    uint64_t masked_val = init_val & mask;
    uint64_t nBytes = calculateNumBytes(nBits);

    for(uint64_t i = 0; i < nBytes; i++){
        uint8_t byte = (masked_val >> (i*8)) & 255;
        PLI_BYTE8 castSafeByte = static_cast<PLI_BYTE8>(byte);
        storage[nBytes - i - 1] = castSafeByte;
    }
    storage[nBytes] = '\0';
}

Signal::Signal(const Signal& other) : Signal(other.nBits){
    uint16_t nBytes = calculateNumBytes(other.nBits);
    std::memcpy(storage, other.storage, nBytes + 1);
}

Signal::Signal(uint16_t nBits, PLI_BYTE8 * init_val) : Signal(nBits){
    auto nBytes = getNumBytes();
    std::memcpy(storage, init_val, nBytes + 1);
}

uint16_t Signal::getNumBits(){
    return nBits;
}

uint16_t Signal::getNumBytes(){
    const uint16_t ret = calculateNumBytes(nBits);
    return ret;
}

template<typename T>
T getUIntScalarHelper(uint16_t nBytes, PLI_BYTE8 * storage){
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

    T ret = getUIntScalarHelper<T>(nBytesStored,storage);
    return ret;
}

uint8_t shiftRMask(uint8_t data, uint8_t mask, int shift){
    auto dataShifted = shift > 0 ? data >> shift : data << (0-shift);
    auto dataShiftedMasked = dataShifted & mask;
    return dataShiftedMasked;
}

template<typename T>
void getUintVectorHelper(T & ret, uint16_t wordSizeBits, int & bitStart, PLI_BYTE8 * storage){
    auto storageByteIdx = bitStart / 8;
    auto localBitStart = bitStart - (storageByteIdx*8);
    auto shift = (8-localBitStart)-wordSizeBits;
    uint8_t mask = (1 << wordSizeBits)-1;
    
    uint8_t word = shiftRMask(storage[storageByteIdx], mask, shift);
    ret |= static_cast<uint8_t>(word);//todo ret > 8
    
    if(shift < 0){
        auto cutoffWordSizeBits = (wordSizeBits-(8-localBitStart));
        auto cutoffBitStart = bitStart + (wordSizeBits-cutoffWordSizeBits);
        getUintVectorHelper(ret, cutoffWordSizeBits, cutoffBitStart, storage);
    }
    bitStart += wordSizeBits;
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

    auto bitStart = 0;
    for(auto i = 0; i<vectorSize; i++){
        getUintVectorHelper<T>(ret[i], wordSizeBits, bitStart, storage);
    }
    return ret;
}

t_vpi_value Signal::getVpiValue(){
    auto nBytes = getNumBytes();
    PLI_BYTE8 * buf = new PLI_BYTE8[nBytes+1];
    std::memcpy(buf,storage,nBytes+1);

    t_vpi_value ret = {vpiStringVal, buf};
    return ret;
}

/*
    0:
    ws = 3
    idx = 0
    bs = 0
    ls = 0
    shift = 5
    mask = 111
    storage[idx]=AAABBBCC
    word = AAA
    ret = AAA
    1:
    ws = 3
    idx = 0
    bs = 3
    ls = 3
    shift = 2
    mask = 111
    storage[idx]=AAABBBCC
    word = BBB
    ret = BBB
    2:
    ws = 3
    idx = 0
    bs = 6
    ls = 6
    shift = -1
    mask = 111
    storage[idx]=AAABBBCC
    word = CC0
    ret = CC0
    2.5:
    ws = 1
    idx = 1
    bs = 8
    ls = 0
    shift = 7
    mask = 1
    storage[idx]=CDDDEEEF
    word = 00C
    ret = CCC
    */