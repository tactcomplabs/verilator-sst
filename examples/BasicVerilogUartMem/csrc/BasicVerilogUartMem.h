#ifndef _BASIC_VERILOG_COUNTER_H_
#define _BASIC_VERILOG_COUNTER_H_

#include <memory>
#include <sst/core/sst_config.h>
#include <sst/core/component.h>
#include "verilatorSST.h"
#include "VTop.h"

namespace SST::VerilatorSST {
struct TestBenchCommand {
    bool transmit;
    int data;
};

class BasicVerilogUartMem : public SST::Component{
    private:
        SST::Output* out;
        std::unique_ptr<VerilatorSST<VTop>> top;
        
        uint16_t baudPeriod;
        uint16_t addrWidth;
        uint16_t dataWidth;
        uint16_t frameWidth;

        enum OpState {IDLE, RECV, INIT_TRANSMIT, TRANSMIT};
        OpState opState = IDLE;
        
        uint64_t rxBuf = 0;
        uint64_t txBuf = 0;
        uint16_t bitCtr = 0;
        uint16_t baudCtr = 0;
        
        uint16_t driverCommandsLen;
        TestBenchCommand * driverCommands;

        uint16_t cmdCtr = 0;
        uint16_t timeout = 0;

        bool clock(SST::Cycle_t cycle);
        void verilatorSetup();
        bool uartDriver();
        void verifyMemory();

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
            { "FRAME_WIDTH", "width of UART frame",                                    "8"    },
            { "ADDR_WIDTH",  "log2 of addressable memory range ",                      "8"    },
            { "DATA_WIDTH",  "data bits at each address location",                     "8"    },
            { "BAUD_PERIOD", "num of clock periods to wait between UART reads/writes", "16"    }
        )

        SST_ELI_DOCUMENT_PORTS()
        SST_ELI_DOCUMENT_STATISTICS()
        SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS()

        BasicVerilogUartMem(SST::ComponentId_t id, SST::Params& params);
        ~BasicVerilogUartMem();

    };
}

#endif
