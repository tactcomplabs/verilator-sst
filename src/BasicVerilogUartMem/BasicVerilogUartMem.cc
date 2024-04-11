#include "BasicVerilogUartMem.h"

#define LOW 0
#define HIGH 1

using namespace SST::VerilatorSST;

BasicVerilogUartMem::BasicVerilogUartMem(ComponentId_t id, Params& params): 
	Component(id) {
	out = new Output("", 1, 0, Output::STDOUT);
	const std::string clockFreq = params.find<std::string>("CLOCK_FREQ", "1GHz");
	baudPeriod = params.find<std::uint16_t>("BAUD_PERIOD");
	dataWidth = params.find<std::uint16_t>("DATA_WIDTH");
	addrWidth = params.find<std::uint16_t>("ADDR_WIDTH");
	
	registerClock(clockFreq, new Clock::Handler<BasicVerilogUartMem>(this, &BasicVerilogUartMem::clock));
	out->output("Registering clock with frequency=%s\n", clockFreq.c_str());
	out->output("Baud period set to baudPeriod=%u\n", baudPeriod);
	out->output("Address width set to addrWidth=%u\n", addrWidth);
	out->output("Data width period set to dataWidth=%u\n", dataWidth);

	verilatorSetup();
	registerAsPrimaryComponent();
	primaryComponentDoNotEndSim();
}

BasicVerilogUartMem::~BasicVerilogUartMem(){
	delete out;
}

void BasicVerilogUartMem::verilatorSetup(){
	top = std::make_unique<VerilatorSST<VUART>>();
	Signal init_low(1,LOW);
	Signal init_high(1,HIGH);
	top->writePort("clk",init_low);
	top->writePort("rst_l",init_low);
	top->writePort("RX",init_high);
	top->writePortAtTick("rst_l",init_high,10);
	top->tick(1);
	driver = std::vector<TestBenchCommand> {
		{true,1},//put mem[0] 7
		{true,0},
		{true,7},
		{true,1},//put mem[1] 6
		{true,1},
		{true,6},
		{true,1},//put mem[2] 5
		{true,2},
		{true,5},
		{true,1},//put mem[3] 4
		{true,3},
		{true,4},
		{true,1},//put mem[4] 3
		{true,4},
		{true,3},
		{true,1},//put mem[5] 2
		{true,5},
		{true,2},
		{true,1},//put mem[6] 1
		{true,6},
		{true,1},
		{true,1},//put mem[7] 7
		{true,7},
		{true,7}
		};
}

void BasicVerilogUartMem::compareMemDebug(uint16_t data, uint16_t addr){
	int bitLength = (1 << addrWidth)*dataWidth;
	Signal memDebug(bitLength);
	top->readPort("mem_debug",memDebug);
	// uint16_t * val = memDebug.getUIntVector<uint16_t>(3);
	for(int i = 0;i<16;i++){
		std::cout << "memDebug["<<i<<"]="<<+static_cast<uint8_t>(memDebug.value.str[i])<<std::endl;
	}
	/*
	111 110 101 100 011 010 001 111
	111 110 101 100 011 010 001 111
	111000000000000000000111
	11100000 00000000 00000111
	*/
};

bool BasicVerilogUartMem::stateMachine(){
	bool ret = false;
	
	if(opState == IDLE){
		std::cout <<driver.size()<<std::endl;
		if(cmdCtr >= driver.size()){
			compareMemDebug(1,1);
			top->finish();
			return true;
		}
		std::cout << "debug" <<std::endl;
		Signal rx;
		top->readPort("TX",rx);
		if((rx.getUIntScalar<uint8_t>() & 1) == LOW){
			rxBuf = 0;
			baudCtr = baudPeriod/2;
			timeout = 0;
			bitCtr = 0;
			opState = RECV;
			out->output("sst: transition IDLE->RECEIVE cmdCtr=%u\n",cmdCtr); 
		}else if(driver[cmdCtr].transmit){
			txBuf = driver[cmdCtr].data | -1 << addrWidth;
			baudCtr = 0;
			bitCtr = 0;
			timeout = 0;
			opState = INIT_TRANSMIT;
			out->output("sst: transition IDLE->INIT_TRANSMIT txBuf=%u cmdCtr=%u\n",txBuf,cmdCtr);
		}
		return ret;
	}

	if(opState == RECV){
		bool allBitsRead = (bitCtr >= addrWidth+2);
		bool doRead = baudCtr >= baudPeriod;
		baudCtr++;
		
		if(allBitsRead && doRead){
			int mask = (1<<dataWidth)-1;
			int maskedRxBuf = (rxBuf >> 1) & mask;
			out->output("sst: received final %u from UART\n", +maskedRxBuf);
			if(driver[cmdCtr].data != maskedRxBuf){
				out->output("sst: bad value data=%u maskedRxBuf=%u rxBuf=%u",driver[cmdCtr].data,maskedRxBuf,rxBuf);
			}
			bitCtr = 0;
			baudCtr = 0;
			cmdCtr++;
			out->output("sst: transition RECV->IDLE cmdCtr=%u\n", cmdCtr);
			opState = IDLE;
		}

		if(!allBitsRead && doRead){
			Signal rx;
			top->readPort("TX",rx);
			int bit = (rx.getUIntScalar<uint8_t>() & 1);
			rxBuf |= (bit & 1) << bitCtr;
			out->output("sst: received %u from UART rxBuf=%u bitCtr=%u\n",bit,rxBuf,bitCtr);
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
			out->output("sst: transition INIT_TRANSMIT->TRANSMIT time=%llu\n",top->getCurrentTick());
		}

		if(!startBitSent){
			Signal tx(1,LOW);
			top->writePort("RX",tx);
			bitCtr++;
			out->output("sst: sent %u to UART time=%llu\n", 0, top->getCurrentTick());
		}
		return ret;
	}

	if(opState == TRANSMIT){
		bool doWrite = baudCtr >= baudPeriod;
		bool stopBitSent = (bitCtr >= addrWidth+1);
		baudCtr++;

		if(stopBitSent && doWrite){
			opState = IDLE;
			bitCtr = 0;
			baudCtr = 0;
			cmdCtr++;
			out->output("sst: transition TRANSMIT->IDLE time=%llu cmdCtr=%u\n", top->getCurrentTick(), cmdCtr);
		}

		if(!stopBitSent && doWrite){
			int bit = txBuf & 1;
			Signal tx(1, bit);
			top->writePort("RX",tx);
			out->output("sst: sent %u to UART time=%llu txBuf=%u bitCtr=%u stopBitSent=%u\n", bit, top->getCurrentTick(), txBuf, bitCtr,stopBitSent);
			txBuf = txBuf >> 1;
			bitCtr++;
			baudCtr = 0;
		}
		return ret;
	}

	assert(false && "bad state");
}

bool BasicVerilogUartMem::clock(Cycle_t cycles){
	if(top->getCurrentTick() > 5){
		if(stateMachine()){
			primaryComponentOKToEndSim();
			return true;
		}
	}

	top->tickClockPeriod("clk");
	return false;
}
