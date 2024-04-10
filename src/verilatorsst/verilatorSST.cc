#ifndef VL_DEBUG
#define VL_DEBUG 0
#endif

#include "verilatorSST.h"

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

    vpi_get_value(vh1, &val);
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

        uint8_t not_clk = ~clk.getUIntValue<uint8_t>();
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
