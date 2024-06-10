#include "verilatorSST.h"
#include <cmath>
using namespace SST::VerilatorSST;

template <class T>
VerilatorSST<T>::VerilatorSST(){
    static_assert(std::is_base_of_v<VerilatedModel,T>);
    contextp = std::make_unique<VerilatedContext>();
    contextp->randReset(2);
    contextp->traceEverOn(true);
    const char* empty {};
    contextp->commandArgs(0, &empty);
    top = std::make_unique<T>(contextp.get(), "");

    signalQueue = std::make_unique<std::vector<SignalQueueEntry>>();
    
    #ifdef DEBUG
    contextp->debug(99);
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
        s_vpi_value val{vpiVectorVal};
        vpi_get_value(vh1, &val);
        
        auto signalFactory = SignalFactory(vpiSizeVal,1);
        auto signalPtr = signalFactory(val);
        Signal signal = std::move(*signalPtr);
        delete signalPtr;
        return signal;
    }

    if(vpiTypeVal == vpiMemory){
        vpiHandle iter = vpi_iterate(vpiMemoryWord,vh1);
        assert(iter);

        SignalFactory * signalFactory;
        Signal * signalPtr = nullptr;
        auto i = 0;
        while(auto rowHandle = vpi_scan(iter)){
            if(i == 0){
                const auto rowSizeBits = vpi_get(vpiSize, rowHandle);
                signalFactory = new SignalFactory(rowSizeBits, vpiSizeVal);
            }

            s_vpi_value row{SIGNAL_VPI_FORMAT};
            vpi_get_value(rowHandle, &row);
            signalPtr = (*signalFactory)(row);

            vpi_free_object(rowHandle);
            i++;
        }

        Signal signal = *signalPtr;
        delete signalPtr;
        delete signalFactory;
        return signal;
    }
    
    assert(false && "unsupported vpiType");
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
        vpi_put_value(vh1,&val,NULL,vpiNoDelay);
        return;
    }

    if(vpiTypeVal == vpiMemory){
        assert(vpiSizeVal == signal.getDepth() && "port depth must match signal depth");
        vpiHandle iter = vpi_iterate(vpiMemoryWord,vh1);
        assert(iter);

        int i = 0;
        while(auto rowHandle = vpi_scan(iter)){
            auto rowSizeBits = vpi_get(vpiSize, rowHandle);
            assert(rowSizeBits == signal.getNumBits() && "row width must match signal width");

            t_vpi_value val = signal.getVpiValue(i);
            vpi_put_value(rowHandle,&val,NULL,0);

            vpi_free_object(rowHandle);
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

        uint8_t not_clk = ~clk.getUIntScalar();
        uint8_t next_clk_byte = not_clk;

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
