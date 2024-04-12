#ifndef VL_DEBUG
#define VL_DEBUG 0
#endif

#include "verilatorSST.h"
#include <iostream>
#include <cmath>
using namespace SST::VerilatorSST;

template <class T>
VerilatorSST<T>::VerilatorSST(){
    static_assert(std::is_base_of_v<VerilatedModel,T>);
    contextp = std::make_unique<VerilatedContext>();
    contextp->debug(VL_DEBUG);
    contextp->randReset(2);
    contextp->traceEverOn(true);
    const char* empty {};
    contextp->commandArgs(0, &empty);
    top = std::make_unique<T>(contextp.get(), "");

    signalQueue = std::make_unique<std::vector<SignalQueueEntry>>();
    
    #if VL_DEBUG
    contextp->internalsDump();
    #endif
}

uint8_t maskShiftL(uint8_t data, uint8_t mask, int shift){
    auto dataMasked = data & mask;
    auto dataMaskedShifted = shift > 0 ? dataMasked << shift : dataMasked >> (0-shift);
    std::cout << "dataMasked=" <<+dataMasked << std::endl; 
    std::cout << "dataMaskedShifted=" <<+dataMaskedShifted << std::endl; 
    return dataMaskedShifted;
}

void readHelper(uint8_t word, uint16_t wordSizeBits, int & bitStart, PLI_BYTE8 * storage){
    auto storageByteIdx = bitStart / 8;
    auto localBitStart = bitStart - (storageByteIdx*8);
    auto shift = (8-localBitStart)-wordSizeBits;
    uint8_t mask = (1 << wordSizeBits)-1;
    storage[storageByteIdx] &= ~(maskShiftL(  -1, mask, shift));
    storage[storageByteIdx] |= maskShiftL(word, mask, shift);

    std::cout << "word=" <<+word << std::endl;                
    std::cout << "wordSizeBits=" << +wordSizeBits<< std::endl;
    std::cout << "bitStart=" << bitStart << std::endl;
    std::cout << "storageByteIdx=" << storageByteIdx<< std::endl;
    std::cout << "localBitStart=" << localBitStart<< std::endl;
    std::cout << "shift=" << shift << std::endl;
    std::cout << "mask=" << +mask << std::endl;
    std::cout << "storage[" << storageByteIdx << "]=" << +static_cast<uint8_t>(storage[storageByteIdx]) << std::endl << std::endl;
    
    if(shift < 0){
        auto cutoffWordSizeBits = (wordSizeBits-(8-localBitStart));
        auto cutoffBitStart = bitStart + (wordSizeBits-cutoffWordSizeBits);
        std::cout << "cutoffWordSizeBits=" << cutoffWordSizeBits << std::endl << std::endl;
        readHelper(word, cutoffWordSizeBits, cutoffBitStart, storage);
    }

    bitStart += wordSizeBits;
}

template <class T>
Signal VerilatorSST<T>::readPort(std::string portName){
    char *name = new char[portName.length() + 1];
    strcpy(name,portName.c_str());

    vpiHandle vh1 = vpi_handle_by_name(name, NULL);
    assert(vh1);

    auto vpiSizeVal = vpi_get(vpiSize, vh1);
    auto vpiTypeVal = vpi_get(vpiType, vh1);

    std::cout << "portName=" << portName << " vpiTypeVal=" << vpiTypeVal << std::endl;

    if(vpiTypeVal != vpiMemory){
        t_vpi_value val{vpiStringVal};
        vpi_get_value(vh1, &val);

        Signal ret(vpiSizeVal,val.value.str);
        return ret;
    }
    if(vpiTypeVal == vpiMemory){
        vpiHandle iter = vpi_iterate(vpiMemoryWord,vh1);
        assert(iter);

        auto bitStart=0;
        auto firstWordSizeBits = 0;
        PLI_BYTE8 * buf;

        while(auto wordHandle = vpi_scan(iter)){
            t_vpi_value word{vpiIntVal};
            vpi_get_value(wordHandle, &word);
            PLI_INT32 wordSizeBits = vpi_get(vpiSize, wordHandle);

            if(bitStart == 0){
                firstWordSizeBits = wordSizeBits;
                buf = new PLI_BYTE8[firstWordSizeBits*vpiSizeVal];
            }
            assert(firstWordSizeBits == wordSizeBits);

            readHelper(word.value.integer, wordSizeBits, bitStart, buf);
            vpi_free_object(wordHandle);
        }
        Signal ret(firstWordSizeBits*vpiSizeVal, buf);
        return ret;
    }
}

template <class T>
void VerilatorSST<T>::writePort(std::string portName, Signal & signal){
    char *name = new char[portName.length() + 1];
    strcpy(name, portName.c_str());
    
    vpiHandle vh1 = vpi_handle_by_name(name, NULL);
    assert(vh1);

    s_vpi_time vpi_time_s;
    vpi_time_s.type = vpiSimTime;
    vpi_time_s.high = 0;
    vpi_time_s.low = 0;
    t_vpi_value val = signal.getVpiValue();
    vpi_put_value(vh1,&val,&vpi_time_s,vpiInertialDelay);
}

template <class T>
void VerilatorSST<T>::writePortAtTick(std::string portName, Signal & signal, uint64_t writeTick){
    assert(writeTick > getCurrentTick());

    SignalQueueEntry entry{portName, writeTick, signal};

    if (signalQueue->empty()){
        signalQueue->push_back(entry);
        return;
    }

    auto i = 0;
    for(auto it : *signalQueue){
        if(it.writeTick < writeTick){
            break;
        }
        i++;
    }
    signalQueue->insert(signalQueue->begin() + i, entry);
}

template <class T>
void VerilatorSST<T>::pollSignalQueue(){
    while(signalQueue->empty() != true){
        if(signalQueue->back().writeTick > getCurrentTick()){
            break;
        }

        writePort(signalQueue->back().portName, signalQueue->back().signal);
        signalQueue->pop_back();
    } 
}

template <class T>
void VerilatorSST<T>::tick(uint64_t elapse){
    assert(!isFinished);
    for(uint64_t i = 0; i < elapse; i++){
        pollSignalQueue();
        contextp->timeInc(1);
        top->eval();
    }
}

template <class T>
void VerilatorSST<T>::tickClockPeriod(std::string clockPort){
    for(auto i = 0; i < 2; i++){
        Signal clk = readPort(clockPort);

        uint8_t not_clk = ~clk.getUIntScalar<uint8_t>();
        uint8_t next_clk_byte = not_clk & 1;

        Signal next_clk(1, next_clk_byte);
        writePort(clockPort, next_clk );

        tick(1);
    }
}

template <class T>
uint64_t VerilatorSST<T>::getCurrentTick(){
    return contextp->time();
}

template <class T>
void VerilatorSST<T>::finish(){
    top->final();
    isFinished = true;
}

    /*
    word0 101
    word1 111
    word2 101
    word3 011
    Storage {101}{111}{10 1}0000000

    wordSizeBits = 3
    bitstart=0
    for(word)
    ...

    wordSizeBits = 3
    while(word)
        word = //{101} on 3rd iteration

        f(word,wordSizeBits,&bitStart,&storage)
            bitstart += wordSizeBits //6
            byteArrayIdx = bitStart / 8 //0
            localbitstart = bitStart - byteArrayIdx*8 //6
            shift = 8-(localBitStart-wordSizeBits) // -1
            mask = (((1<<wordSizeBits)-1) << (8-wordSizeBits)) >> localBitStart //0000 00011
            mask = (1-wordSizeBits)-1 //            0000 0111
            
            wordMasked = word & mask //             0000 0101
            wordShifted = word << shift             0000 0010
            storage[0] &= ~(mask << shift)//        1011 1100
            storage[0] |= wordShifted //            1011 1110

            if(shift < 0)
                WordSizeBits = (wordSizeBits-(8-localBitStart)) // 1
                f(word,wordSizeBits,&bitStart,&storage)
                    BitStart += WordSizeBits //9
                    ByteArrayIdx = BitStart / 8 //1
                    LocalBitStart= BitStart-ByteArrayIdx*8 //0
                    Shift = 8-LocalBitStart-WordSizeBits //7
                    Mask = (1-WordSizeBits)-1 //             0000 0001
                    WordMasked = Word & Mask //         0000 0001
                    WordShifted = WordMasked << Shift //1000 0000
                    storage[1] &= ~(Mask << Shift) //       0000 0000
                    storage[1] &= ~(Mask << Shift) //       1000 0000


    word=011
    wordSizeBits=3
    bitStart += wordSizeBits
    
    word     =    01010101
    wordMask =    00000111
    wordMasked =  00000101
    wordShifted = 00010100 = wordMasked <<shift

    storage =     10101010
    storageMask = 00011100 = wordMask << shift
    rdy_storage = 10100010 = storage &= ~storageMask
    good_storage = 10110110 
*/