#ifndef VL_DEBUG
#define VL_DEBUG 0
#endif

#include "verilatorSST.h"
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
Signal VerilatorSST<T>::readPort(std::string portName){
    char *name = new char[portName.length() + 1];
    strcpy(name,portName.c_str());

    vpiHandle vh1 = vpi_handle_by_name(name, NULL);
    assert(vh1 && "vpi should return a handle (port not found)");

    auto vpiTypeVal = vpi_get(vpiType, vh1);
    auto vpiSizeVal = vpi_get(vpiSize, vh1);

    if(vpiTypeVal == vpiReg){
        t_vpi_value val{vpiStringVal};
        vpi_get_value(vh1, &val);

        Signal ret(vpiSizeVal,1,val.value.str);
        return ret;
    }

    if(vpiTypeVal == vpiMemory){
        vpiHandle iter = vpi_iterate(vpiMemoryWord,vh1);
        assert(iter);

        bool first = true;
        auto firstWordSizeBits = 0;
        auto nBytesPerWord = 0;
        auto totalBytes = 0;
        PLI_BYTE8 * buf;

        int i = 0;
        while(auto wordHandle = vpi_scan(iter)){
            PLI_INT32 wordSizeBits = vpi_get(vpiSize, wordHandle);

            if(first == true){
                firstWordSizeBits = wordSizeBits;
                nBytesPerWord = Signal::calculateNumBytes(firstWordSizeBits);
                totalBytes = nBytesPerWord*vpiSizeVal;
                buf = new PLI_BYTE8[nBytesPerWord*vpiSizeVal];
                first = false;
            }
            assert(firstWordSizeBits == wordSizeBits);

            t_vpi_value word{vpiStringVal};
            vpi_get_value(wordHandle, &word);

            auto offset = (totalBytes-(nBytesPerWord*(1+i)));
            std::memcpy(buf+offset,word.value.str,nBytesPerWord);

            vpi_free_object(wordHandle);
            i++;
        }

        Signal ret(firstWordSizeBits, i, buf);
        return ret;
    }
    
    assert(false && "unsupported vpiType");
    return NULL;
}

template <class T>
void VerilatorSST<T>::writePort(std::string portName, Signal & signal){
    char *name = new char[portName.length() + 1];
    strcpy(name, portName.c_str());
    
    vpiHandle vh1 = vpi_handle_by_name(name, NULL);
    assert(vh1 && "vpi should return a handle (port not found)");

    auto vpiTypeVal = vpi_get(vpiType, vh1);
    auto vpiSizeVal = vpi_get(vpiSize, vh1);
    
    auto vpiDirVal = vpi_get(vpiDirection,vh1);
    assert(vpiDirVal == vpiInput && "port must be an input, inout not supported");

    if(vpiTypeVal == vpiReg){
        assert(vpiSizeVal == signal.getNumBits() && "port width must match signal width");
        t_vpi_value val = signal.getVpiValue(0);
        vpi_put_value(vh1,&val,NULL,0);
        return;
    }

    if(vpiTypeVal == vpiMemory){
        assert(vpiSizeVal == signal.getDepth() && "port depth must match signal depth");
        vpiHandle iter = vpi_iterate(vpiMemoryWord,vh1);
        assert(iter);

        int i = 0;
        while(auto wordHandle = vpi_scan(iter)){
            auto wordSizeBits = vpi_get(vpiSize, wordHandle);
            assert(wordSizeBits == signal.getNumBits() && "word width must mach signal width");

            t_vpi_value val = signal.getVpiValue(i);
            vpi_put_value(wordHandle,&val,NULL,0);

            vpi_free_object(wordHandle);
            i++;
        }

        return;
    }
    assert(false && "unsupported vpiType");
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
