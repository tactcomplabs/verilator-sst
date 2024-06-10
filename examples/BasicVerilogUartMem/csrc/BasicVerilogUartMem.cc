#include "BasicVerilogUartMem.h"

using namespace SST::VerilatorSST;

BasicVerilogUartMem::BasicVerilogUartMem(ComponentId_t id, Params& params): 
	Component(id) {
	out = new Output("", 1, 0, Output::STDOUT);
	const std::string clockFreq = params.find<std::string>("CLOCK_FREQ", "1GHz");
	baudPeriod = params.find<std::uint16_t>("BAUD_PERIOD");
	dataWidth = params.find<std::uint16_t>("DATA_WIDTH");
	addrWidth = params.find<std::uint16_t>("ADDR_WIDTH");
	frameWidth = params.find<std::uint16_t>("FRAME_WIDTH");
	
	registerClock(clockFreq, new Clock::Handler<BasicVerilogUartMem>(this, &BasicVerilogUartMem::clock));
	out->output("Registering clock with frequency=%s\n", clockFreq.c_str());
	out->output("Baud period set to baudPeriod=%u\n", baudPeriod);
	out->output("Address width set to addrWidth=%u\n", addrWidth);
	out->output("Data width period set to dataWidth=%u\n", dataWidth);
	out->output("Frame width period set to frameWidth=%u\n", frameWidth);

	verilatorSetup();
	registerAsPrimaryComponent();
	primaryComponentDoNotEndSim();
}

BasicVerilogUartMem::~BasicVerilogUartMem(){
	delete out;
	delete[] driverCommands;
}

void BasicVerilogUartMem::verilatorSetup(){
	top = std::make_unique<VerilatorSST<VTop>>();

	Signal init_low(1,SIGNAL_LOW);
	Signal init_high(1,SIGNAL_HIGH);

	top->writePort("clk",init_low);
	top->writePort("rst_l",init_low);
	top->writePort("RX",init_high);
	top->writePortAtTick("rst_l",init_high,10);
	top->tick(1);

	const auto addrLen = (1 << addrWidth);
	const auto maxAddr = addrLen-1;
	driverCommandsLen = addrLen*3*2;
	driverCommands = new TestBenchCommand[driverCommandsLen];

	for(auto i = 0; i<addrLen;i++){
		const auto idx = i*3;
		driverCommands[idx+0] = {true,1};
		driverCommands[idx+1] = {true,i};
		driverCommands[idx+2] = {true,maxAddr-i};
	}

	for(auto i =0; i<addrLen;i++){
		const auto idx = (driverCommandsLen/2)+(i*3);

		driverCommands[idx+0] = {true,0};
		driverCommands[idx+1] = {true,i};
		driverCommands[idx+2] = {false,maxAddr-i};
	}
}

void BasicVerilogUartMem::verifyMemory(){
	Signal mem_debug = top->readPort("mem_debug");
	uint8_t * memDebugArr = mem_debug.getUIntArray(true);

	auto memDebugIdx = 0;
	auto dataBytes = Signal::calculateNumBytes(dataWidth);
	auto addrLen = (1 << addrWidth);
	auto maxAddr = addrLen - 1;

	for(auto i=0;i<addrLen;i++){
		auto mappedAddr = i*dataBytes;
		auto expectedData = maxAddr - i;
		auto data = memDebugArr[mappedAddr];

		assert(data == expectedData && "mem_debug mismatch");
	}
};

bool BasicVerilogUartMem::uartDriver(){
	bool ret = false;
	
	if(opState == IDLE){
		if(cmdCtr >= driverCommandsLen){
			out->output("sst: reached end of driver commands, verifying memory...\n");
			verifyMemory();
			out->output("sst: all tests passed!\n");
			top->finish();
			return true;
		}

		Signal rx = top->readPort("TX");
		if(rx.getUIntScalar() == SIGNAL_LOW){
			rxBuf = 0;
			baudCtr = baudPeriod/2;
			timeout = 0;
			bitCtr = 0;
			opState = RECV;
			out->output("sst: transition IDLE->RECEIVE cmdCtr=%u\n",cmdCtr); 
		}else if(driverCommands[cmdCtr].transmit){
			txBuf = driverCommands[cmdCtr].data | -1 << frameWidth;
			baudCtr = 0;
			bitCtr = 0;
			timeout = 0;
			opState = INIT_TRANSMIT;
			out->output("sst: transition IDLE->INIT_TRANSMIT txbuf=%lu cmdCtr=%u\n",txBuf,cmdCtr);
		}
		return ret;
	}

	if(opState == RECV){
		bool allBitsRead = (bitCtr >= frameWidth+2);
		bool doRead = baudCtr >= baudPeriod;
		baudCtr++;
		
		if(allBitsRead && doRead){
			int mask = (1<<dataWidth)-1;
			int maskedRxBuf = (rxBuf >> 1) & mask;
			out->output("sst: received data %u from UART\n", +maskedRxBuf);
			assert(driverCommands[cmdCtr].data == maskedRxBuf && "data mismatch");
			bitCtr = 0;
			baudCtr = 0;
			cmdCtr++;
			out->output("sst: transition RECV->IDLE cmdCtr=%u\n", cmdCtr);
			opState = IDLE;
		}

		if(!allBitsRead && doRead){
			Signal tx = top->readPort("TX");
			auto bit = tx.getUIntScalar();
			rxBuf |= bit << bitCtr;
			out->output("sst: received bit %u from UART rxbuf=%lu bitCtr=%u\n",bit,rxBuf,bitCtr);
			bitCtr++;
			baudCtr = 0;
		}

		return ret;
	}

	if(opState == INIT_TRANSMIT){
		bool doWrite = baudCtr >= baudPeriod;
		bool startBitSent = (bitCtr > 0);
		baudCtr++;

		if(startBitSent && doWrite){
			opState = TRANSMIT;
			bitCtr = 0;
			out->output("sst: transition INIT_TRANSMIT->TRANSMIT time=%lu\n",top->getCurrentTick());
		}

		if(!startBitSent){
			Signal tx(1,SIGNAL_LOW);
			top->writePort("RX",tx);
			bitCtr++;
			out->output("sst: sent start bit %u to UART time=%lu\n", 0, top->getCurrentTick());
		}
		return ret;
	}

	if(opState == TRANSMIT){
		bool doWrite = baudCtr >= baudPeriod;
		bool stopBitSent = (bitCtr >= frameWidth+1);
		baudCtr++;

		if(stopBitSent && doWrite){
			opState = IDLE;
			bitCtr = 0;
			baudCtr = 0;
			cmdCtr++;
			out->output("sst: transition TRANSMIT->IDLE time=%lu cmdCtr=%u\n", top->getCurrentTick(), cmdCtr);
		}

		if(!stopBitSent && doWrite){
			auto bit = txBuf & 1;
			Signal tx(1, bit);
			top->writePort("RX",tx);
			out->output("sst: sent frame bit %lu to UART time=%lu txbuf=%lu bitCtr=%u stopBitSent=%u\n", bit, top->getCurrentTick(), txBuf, bitCtr,stopBitSent);
			txBuf = txBuf >> 1;
			bitCtr++;
			baudCtr = 0;
		}
		return ret;
	}

	assert(false && "bad state");
};

bool BasicVerilogUartMem::clock(Cycle_t cycles){
	if(top->getCurrentTick() > 5){
		if(uartDriver()){
			primaryComponentOKToEndSim();
			return true;
		}
	}

	top->tickClockPeriod("clk");
	return false;
};
