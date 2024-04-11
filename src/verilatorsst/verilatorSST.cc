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

template <class T>
void VerilatorSST<T>::readPort(std::string portName, Signal & val){
    char *name = new char[portName.length() + 1];
    strcpy(name,portName.c_str());

    vpiHandle vh1 = vpi_handle_by_name(name, NULL);
    assert(vh1);

    PLI_INT32 vpiSizeVal = vpi_get(vpiSize, vh1);
    PLI_INT32 vpiTypeVal = vpi_get(vpiType, vh1);

    std::cout << "portName=" << portName << " vpiTypeVal=" << vpiTypeVal << std::endl;

    std::cout << "val.getNumBytes()=" <<val.getNumBytes()<<std::endl;
    std::cout << "vpiSizeVal=" <<vpiSizeVal<<std::endl;
    // auto wordSizeBytes = val.getNumBytes() / vpiSizeVal;

    if(vpiTypeVal == vpiMemory){
        vpiHandle iter = vpi_iterate(vpiMemoryWord,vh1);
        assert(iter);
        int i= 0;
        int bitStart = 0;
        while(auto wordHandle = vpi_scan(iter)){
            t_vpi_value word{vpiIntVal};
            vpi_get_value(wordHandle, &word);
            PLI_INT32 wordSizeBits = vpi_get(vpiSize, wordHandle);
            std::cout << "wordSizeBits=" <<wordSizeBits<<std::endl;
            auto bitStart = i*wordSizeBits;
            // auto bitEnd = bitStart+wordSizeBits-1;
            std::cout << "bitStart=" <<bitStart<<std::endl;
            auto byteArrayIdx = bitStart / 8;
            std::cout << "byteArrayIdx=" <<byteArrayIdx<<std::endl;
            // bool misaligned = bitEnd / 8 != byteArrayIdx;
            // assert(!misaligned);
            auto localBitStart = bitStart - byteArrayIdx*8;
            std::cout << "localBitStart=" <<localBitStart<<std::endl;
            auto shift = 8-localBitStart-wordSizeBits;
            std::cout << "shift=" <<shift<<std::endl;
            assert(shift >= 0 && "word size is not a factor of 8");
            uint8_t mask = (((1<<wordSizeBits)-1) << (8-wordSizeBits)) >> localBitStart;
            std::cout << "mask=" <<+mask<<std::endl;
            if(shift < 0 ){
                //todo
            }else{
                uint8_t wordShifted = word.value.integer << shift;
                std::cout << "word.value.integer=" <<+word.value.integer<<std::endl;
                uint8_t wordMasked = wordShifted & mask;
                std::cout << "wordMasked=" <<+wordMasked<<std::endl;
                val.value.str[byteArrayIdx] &= ~mask;
                val.value.str[byteArrayIdx] |= wordMasked;
                std::cout << "val.value.str[byteArrayIdx]=" <<+val.value.str[byteArrayIdx]<<std::endl;
            }
            

            //word0 = 00000101
            //word1 = 00000001
            //word2 = 00000110
            //word4 = 00000001
            //data =  10100110


            //00000000
            //11100000
            //00000011 1
            //11111111 

            //i=0
            //wordSizeBits = 3
            //bitstart = i*wordSizeBits
            //byteArrayIdx = bitstart // 8
            //bitend = (i+1)*wordSizeBits-1
            //misaligned = bitend // 8 != byteArrayIdx
            //fail for now
            //localBitStart = bitStart - byteArrayIdx*8

            //mask = (1<<wordSizeBits)-1 << 
            //localBitStart = 0; bitStart = 0 -0 byteArrayIdx*8
            //localBitStart = 3; bitStart = 3 -0
            //localBitStart = 6; bitStart = 6 -0
            //localBitStart = 1; bitStart = 9 -8
            //localBitStart = 4; bitStart = 12 -8
            //localBitStart = 7; bitStart = 15 -8
            //localBitStart = 2; bitStart = 18 -16
            //localBitStart = 5; bitStart = 21 -16
            //localBitStart = 8; bitStart = 24 -16
            //bitstart = 0, byt
            vpi_free_object(wordHandle);
            i++;
        }
    }else {
        assert(val.getNumBits() >= vpiSizeVal);
        vpi_get_value(vh1, &val);
    }
    
}

template <class T>
void VerilatorSST<T>::writePort(std::string portName, Signal & val){
    char *name = new char[portName.length() + 1];
    strcpy(name, portName.c_str());
    
    vpiHandle vh1 = vpi_handle_by_name(name, NULL);
    assert(vh1);

    s_vpi_time vpi_time_s;
    vpi_time_s.type = vpiSimTime;
    vpi_time_s.high = 0;
    vpi_time_s.low = 0;
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
    for(uint64_t i = 0; i < elapse; i++){
        pollSignalQueue();
        contextp->timeInc(1);
        top->eval();
    }
}

template <class T>
void VerilatorSST<T>::tickClockPeriod(std::string clockPort){
    for(auto i = 0; i < 2; i++){
        Signal clk;
        readPort(clockPort, clk);

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
}
