#include "Signal.h"

using namespace SST::VerilatorSST;

signal_width_t Signal::calculateNumBytes(signal_width_t nBits){
    assert(nBits > 0 && "nBits must be positive");
    const signal_width_t ret = 1+((nBits-1)/8);
    return ret;
}

Signal::Signal(signal_width_t nBits) : nBits(nBits), depth(1){
    assert(nBits <= SIGNAL_BITS_MAX);

    signal_width_t nBytes = calculateNumBytes(nBits);
    storage = new PLI_BYTE8[nBytes];
}

Signal::Signal(signal_width_t nBits, uint64_t init_val) : Signal(nBits){
    assert(nBits < 64);

    uint64_t mask = (static_cast<uint64_t>(1) << nBits) - 1;
    uint64_t masked_val = init_val & mask;
    signal_width_t nBytes = calculateNumBytes(nBits);

    for(uint64_t i = 0; i < nBytes; i++){
        uint8_t byte = (masked_val >> (i*8)) & 255;
        PLI_BYTE8 castSafeByte = static_cast<PLI_BYTE8>(byte);
        storage[nBytes - i - 1] = castSafeByte;
    }
}

Signal::Signal(const Signal& other) : Signal(other.nBits){
    signal_width_t nBytes = calculateNumBytes(other.nBits);
    std::memcpy(storage, other.storage, nBytes);
}

Signal::Signal(signal_width_t nBitsPerWord, signal_depth_t depth, PLI_BYTE8 * init_val){
    assert(nBitsPerWord <= SIGNAL_BITS_MAX);
    assert(depth > 0 && "depth must be positive");

    nBits = nBitsPerWord;
    this->depth = depth;

    auto nBytes = getNumBytes();
    storage = new PLI_BYTE8[nBytes*depth];

    //TODO https://github.com/verilator/verilator/issues/5036
    // std::memcpy(storage, init_val, nBytes*depth);
    for(auto i=0;i<(nBytes*depth);i++){
        uint8_t castSafeByte = static_cast<uint8_t>(init_val[i]);
        uint8_t padSafeByte = (castSafeByte == ' ') ? 0 : castSafeByte;
        storage[i] = padSafeByte;
    }
    
}

Signal::Signal(signal_width_t nBitsPerWord, signal_depth_t depth, uint64_t * init_val){
    assert(nBitsPerWord <= SIGNAL_BITS_MAX);
    assert(depth > 0 && "depth must be positive");

    nBits = nBitsPerWord;
    this->depth = depth;

    auto nBytesPerWord = getNumBytes();
    storage = new PLI_BYTE8[nBytesPerWord*depth];

    for(auto i=0; i<depth; i++){
        uint64_t word = init_val[i];

        for(auto j=0; j<nBytesPerWord; j++){
            uint64_t mask = (1<<nBitsPerWord)-1;
            auto shift = (nBytesPerWord-j-1)*8;

            uint64_t wordMasked = word & mask;
            uint64_t wordMaskedShifted = wordMasked >> shift;
            uint8_t safeByte = wordMaskedShifted & 255;

            auto storageIdx = (i*nBytesPerWord) + j;
            storage[storageIdx] = safeByte;
        }
        
    }
}

signal_width_t Signal::getNumBits(){
    return nBits;
}

signal_width_t Signal::getNumBytes(){
    signal_width_t ret = calculateNumBytes(nBits);
    return ret;
}

signal_depth_t Signal::getDepth(){
    return depth;
}



t_vpi_value Signal::getVpiValue(signal_depth_t index){
    assert(index < getDepth() && "index out of range");

    auto nBytes = getNumBytes();
    auto offset = nBytes*(depth - (1+index));

    PLI_BYTE8 * buf = new PLI_BYTE8[nBytes+1];
    std::memcpy(buf,storage+offset,nBytes);
    buf[nBytes] = '\0';

    t_vpi_value ret = {vpiStringVal, buf};
    return ret;
}


