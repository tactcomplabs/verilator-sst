#ifndef _BASIC_VERILOG_COUNTER_H_
#define _BASIC_VERILOG_COUNTER_H_

#include <memory>
#include <sst/core/sst_config.h>
#include <sst/core/component.h>
#include "verilatorSST.h"
#include "verilatorSST.cc"
#include "VUART.h"

namespace SST::VerilatorSST {
struct TestBenchCommand {
    bool transmit;
    int data;
};

class BasicVerilogUartMem : public SST::Component{
    private:
        uint16_t baudPeriod;
        uint16_t addrWidth;
        uint16_t dataWidth;

        uint16_t baudCtr = 0;
        SST::Output* out;
        std::unique_ptr<VerilatorSST<VUART>> top;
        bool clock(SST::Cycle_t cycle);
        void verilatorSetup();
        bool stateMachine();
        void compareMemDebug(uint16_t data, uint16_t addr);
        enum OpState {IDLE, RECV, INIT_TRANSMIT, TRANSMIT, TRANSMIT_STOP};
        OpState opState = IDLE;
        uint16_t rxBuf = 0;
        uint16_t txBuf = 0;
        uint16_t bitCtr = 0;
        
        TestBenchCommand driver[12] = {
            {true,1},//put mem[0] 7
            {true,0},
            {true,7},
            {true,0},//get mem[0]
            {true,0},
            {false,7},
            {true,1},//put mem[15] 7
            {true,15},
            {true,7},
            {true,0},//get mem[15]
            {true,15},
            {false,7}
            };
        uint16_t cmdCtr = 0;
        uint16_t cmdsSize = 12;
        uint16_t timeout = 0;

    public:
        SST_ELI_REGISTER_COMPONENT(
            BasicVerilogUartMem,                               // Component class
            "verilatorsst",                         // Component library
            "BasicVerilogUartMem",                             // Component name
            SST_ELI_ELEMENT_VERSION(1,0,0),           // Version of the component
            "BasicVerilogUartMem: UART connected memory",   // Description of the component
            COMPONENT_CATEGORY_UNCATEGORIZED          // Component category
        )

        SST_ELI_DOCUMENT_PARAMS(
            { "CLOCK_FREQ",  "Frequency of period (with units) of the clock",          "1GHz" },
            { "ADDR_WIDTH",  "log2 of addressable memory range ",                      "8"    },
            { "DATA_WIDTH",  "data bits at each address location",                     "8"    },
            { "BAUD_PERIOD", "num of clock periods to wait between UART reads/writes", "8"    }
        )

        SST_ELI_DOCUMENT_PORTS()
        SST_ELI_DOCUMENT_STATISTICS()
        SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS()

        BasicVerilogUartMem(SST::ComponentId_t id, SST::Params& params);
        ~BasicVerilogUartMem();

    };
}

#endif