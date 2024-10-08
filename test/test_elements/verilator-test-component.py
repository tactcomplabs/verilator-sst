#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#
# verilator-test-component.py
#

import sst
import argparse
import queue
import random
from enum import Enum
from enum import IntEnum

INOUT_PORT = "3"
WRITE_PORT = "2"
READ_PORT = "1"
UINT64_MAX = 0xffff_ffff_ffff_ffff
SCRATCH_ADDR_BASE = 0x0300_0000_0000_0000
SCRATCH_SIZE = 512 * 1024
UART_ADDR_WIDTH = 2
UART_DATA_WIDTH = 2
UART_BAUD_PERIOD = 4 # Should be one more than the verilog param 
# NOTE: Because the verilog seems to have a spare cycle; also if 
# baud period is lower than 4, may have to give special care to 
# the start bit interactions

class OpAction(Enum):
    Write = "write"
    Read  = "read"

class VerboseMasking(IntEnum):
    WRITE_EVENT  = 0b0000_0000_0001
    READ_EVENT   = 0b0000_0000_0010
    ALL_EVENTS   = 0b0000_0000_0011
    WRITE_PORT   = 0b0000_0000_0100
    READ_PORT    = 0b0000_0000_1000
    INOUT_PORT   = 0b0000_0001_0000
    ALL_PORTS    = 0b0000_0001_1100
    READ_DATA    = 0b0000_0010_0000
    WRITE_DATA   = 0b0000_0100_0000
    ALL_DATA     = 0b0000_0110_0000
    TEST_OP      = 0b0000_1000_0000
    WRITE_QUEUE  = 0b0001_0000_0000
    INIT         = 0b0010_0000_0000
    FULL         = 0b0011_1111_1111

class PortDef:
    """ Wrapper class to make port definitions cleaner """
    def __init__(self):
        self.PortList = [ ]
        self.PortId = 0
        self.PortNames = [ ]

    # portName is a string, portSize is an int (measured in bytes), portDir
    # should use READ_PORT, WRITE_PORT, or INOUT_PORT globals
    def addPort(self, portName, portSize, portDir):
        tmp = f"{portName}:{self.PortId}:{portSize}:{portDir}"
        self.PortList.append(tmp)
        self.PortNames.append(portName)
        self.PortId = self.PortId + 1

    def getPortMap(self):
        return(self.PortList)

    def getNumPorts(self):
        return( len(self.PortList) )

    def getPortName(self, index):
        return( self.PortNames[index] )


def randIntBySize(size):
    tmp = 2**(size*8) - 1
    return(random.randrange(tmp))

class Test:
    """ Class for building and exporting tests """
    def __init__(self):
        self.TestOps = [ ]
        self.randValues = queue.Queue()
        self.directMode = 0
        # NOTE: direct mode ignores clk writes

    # Used for test ops that have a value <=64bit
    def addTestOp(self, portName, action, value, tick):
        if (self.directMode == 0 or portName != "clk"):
            tmp = portName + ":" + action.value + ":" + str(value) + ":" + str(tick)
            self.TestOps.append(tmp)

    # Used for test ops that have values >64bit, requires a list of 64 bit values
    def addBigTestOp(self, portName, action, values, tick):
        tmp = portName + ":" + action.value + ":"
        for val in values:
            tmp += str(val) + ":"
        tmp += str(tick)
        self.TestOps.append(tmp)

    def setDirectMode(self):
        self.directMode = 1

    def buildScratchTest(self, numCycles):
        global SCRATCH_ADDR_BASE
        global SCRATCH_SIZE
        for i in range(numCycles):
            self.addTestOp("clk", OpAction.Write, 1, i) # cycle clock every cycle
            # setup different params based on cycles
            if (i % 20 == 1):
                length = 3
                sizeToWrite = 8
            elif (i % 20 == 6):
                length = 2
                sizeToWrite = 4
            elif (i % 20 == 11):
                length = 1
                sizeToWrite = 2
            elif (i % 20 == 16):
                length = 0
                sizeToWrite = 1
            # setup different operations based on cycles (overlapping with the params)
            if (i % 5 == 1):
                self.addTestOp("write", OpAction.Write, 1, i)
                randAddr = randIntBySize(8) % SCRATCH_SIZE # address is always 64 bit
                self.randValues.put(randAddr)
                self.addTestOp("addr", OpAction.Write, SCRATCH_ADDR_BASE + randAddr, i)
                self.addTestOp("len", OpAction.Write, length, i)
                randData = randIntBySize(sizeToWrite)
                self.randValues.put(randData)
                self.addTestOp("wdata", OpAction.Write, randData, i)
                self.addTestOp("en", OpAction.Write, 1, i)
            elif (i % 5 == 2):
                self.addTestOp("en", OpAction.Write, 0, i)
            elif (i % 5 == 3):
                self.addTestOp("write", OpAction.Write, 0, i)
                randAddr = self.randValues.get()
                self.addTestOp("addr", OpAction.Write, SCRATCH_ADDR_BASE + randAddr, i)
                self.addTestOp("len", OpAction.Write, length, i)
                self.addTestOp("en", OpAction.Write, 1, i)
            elif (i % 5 == 4):
                randData = self.randValues.get()
                self.addTestOp("rdata", OpAction.Read, randData, i)
                self.addTestOp("en", OpAction.Write, 0, i)
            self.addTestOp("clk", OpAction.Write, 0, i)

    def buildAccum1DTest(self, numCycles):
        global UINT64_MAX
        self.addTestOp("reset_l", OpAction.Write, 0, 1)
        self.addTestOp("reset_l", OpAction.Write, 1, 3)
        self.addTestOp("clk", OpAction.Write, 1, 3)
        self.addTestOp("clk", OpAction.Write, 0, 3)
        accum = [0, 0, 0, 0]
        bigAccum = 0
        for i in range(4, numCycles):
            self.addTestOp("clk", OpAction.Write, 1, i) # cycle clock every cycle
            if (i % 3 == 1):
                randDataLower = randIntBySize(8)
                randDataUpper = randIntBySize(8)
                bigRandData = (randDataUpper << 64) + randDataLower
                bigAccum += bigRandData
                bigAdd = [randDataLower, randDataUpper]
                accum[0] = bigAccum & 0x0000_ffff_ffff_ffff_ffff
                accum[1] = (bigAccum >> 64) & 0x0000_ffff_ffff_ffff_ffff
                accum[2] = (bigAccum >> 128) & 0x0000_ffff_ffff_ffff_ffff
                accum[3] = (bigAccum >> 192) & 0x0000_ffff_ffff_ffff_ffff
                self.addBigTestOp("add", OpAction.Write, bigAdd, i)
                self.addTestOp("en", OpAction.Write, 1, i)
            elif (i % 3 == 2):
                self.addBigTestOp("accum", OpAction.Read, accum, i)
                self.addTestOp("done", OpAction.Read, 1, i)
                self.addTestOp("en", OpAction.Write, 0, i)
            self.addTestOp("clk", OpAction.Write, 0, i) # cycle clock every cycle
     
    def buildAccumTest(self, numCycles):
        global UINT64_MAX
        self.addTestOp("reset_l", OpAction.Write, 1, 0)
        self.addTestOp("reset_l", OpAction.Write, 0, 1)
        self.addTestOp("reset_l", OpAction.Write, 1, 3)
        self.addTestOp("clk", OpAction.Write, 1, 3)
        self.addTestOp("clk", OpAction.Write, 0, 3)
        accum = [0, 0, 0, 0]  # 32 bit each
        bigAccum = [0, 0] # 64 bit each
        add = [0, 0, 0, 0] # 16 bit each
        bigAdd = 0 # 64 bits
        for i in range(4, numCycles):
            self.addTestOp("clk", OpAction.Write, 1, i) # cycle clock every cycle
            if (i % 3 == 1):
                add[0] = randIntBySize(2)
                add[1] = randIntBySize(2)
                add[2] = randIntBySize(2)
                add[3] = randIntBySize(2)
                bigAdd = (add[3] << 48) + (add[2] << 32) + (add[1] << 16) + add[0]
                accum[0] += add[0]
                accum[1] += add[1]
                accum[2] += add[2]
                accum[3] += add[3]
                bigAccum[0] = accum[0] + (accum[1] << 32)
                bigAccum[1] = accum[2] + (accum[3] << 32)
                self.addTestOp("add", OpAction.Write, bigAdd, i)
                self.addTestOp("en", OpAction.Write, 1, i)
            elif (i % 3 == 2):
                self.addBigTestOp("accum", OpAction.Read, bigAccum, i)
                self.addTestOp("done", OpAction.Read, 1, i)
                self.addTestOp("en", OpAction.Write, 0, i)
            self.addTestOp("clk", OpAction.Write, 0, i) # cycle clock every cycle

    def buildCounterTest(self, numCycles, resetDelay):
        self.addTestOp("reset_l", OpAction.Write, 1, 0)
        self.addTestOp("reset_l", OpAction.Write, 0, 1)
        stopCycle = 4
        currStopVal = 0
        self.addTestOp("clk", OpAction.Write, 1, 3)
        self.addTestOp("clk", OpAction.Write, 0, 3)
        if (resetDelay == 0):
            self.addTestOp("reset_l", OpAction.Write, 1, 3+resetDelay)
            self.addTestOp("stop", OpAction.Write, currStopVal, 3+resetDelay)
        for i in range(4, numCycles):
            self.addTestOp("clk", OpAction.Write, 1, i) # cycle clock every cycle
            if (resetDelay != 0 and i == 4):
                self.addTestOp("reset_l", OpAction.Write, 1, 3+resetDelay)
                self.addTestOp("stop", OpAction.Write, currStopVal, 3+resetDelay)
            if (i < 11+resetDelay):
                if (i < 11):
                    currStopVal += 1
                    self.addTestOp("done", OpAction.Read, 1, i)
                self.addTestOp("stop", OpAction.Write, currStopVal, i)
                stopCycle = i + 8
            elif (i == stopCycle):
                stopCycle = i + 8
                self.addTestOp("done", OpAction.Read, 1, i)
            else:
                self.addTestOp("done", OpAction.Read, 0, i)
            self.addTestOp("clk", OpAction.Write, 0, i) # cycle clock every cycle

    def buildPinTest(self, numCycles):
        def send_mode(cycle):
            send_data = 0xaa
            self.addTestOp("direction", OpAction.Write, 1, cycle)
            self.addTestOp("data_write", OpAction.Write, send_data, cycle)
            self.addTestOp("clk", OpAction.Write, 0, cycle)
            self.addTestOp("clk", OpAction.Write, 1, cycle)
            return send_data
          
        def check_io_port(cycle,expected_data):
            self.addTestOp("io_port", OpAction.Read, expected_data, cycle)
            self.addTestOp("clk", OpAction.Write, 0, cycle)
            self.addTestOp("clk", OpAction.Write, 1, cycle)

        def recv_mode(cycle):
            recv_data = 0xbb
            self.addTestOp("direction", OpAction.Write, 0, cycle)
            self.addTestOp("io_port", OpAction.Write, recv_data, cycle)
            self.addTestOp("clk", OpAction.Write, 0, cycle)
            self.addTestOp("clk", OpAction.Write, 1, cycle)
            return recv_data
          
        def check_data_read(cycle, expected_data):
            self.addTestOp("data_read", OpAction.Read, expected_data, cycle)
            self.addTestOp("clk", OpAction.Write, 0, cycle)
            self.addTestOp("clk", OpAction.Write, 1, cycle)
          
        # 1 iteration through all IO pin modes takes 4 cycles
        numIterations = numCycles // 4
        for iteration in range(1,numIterations):
             currentCycle = iteration*4

             expected_data = send_mode(currentCycle)
             check_io_port(currentCycle+1, expected_data)
             expected_data = recv_mode(currentCycle+2)
             check_data_read(currentCycle+3, expected_data)


    def buildUartTest(self, numCycles):
        global UART_BAUD_PERIOD
        global UART_ADDR_WIDTH
        global UART_DATA_WIDTH

        def initUart():
            self.addTestOp("rst_l", OpAction.Write, 1, 0)
            self.addTestOp("rst_l", OpAction.Write, 0, 0)
            self.addTestOp("RX", OpAction.Write, 1, 1)
            self.addTestOp("rst_l", OpAction.Write, 1, 3)
            self.addTestOp("clk", OpAction.Write, 0, 3)
            self.addTestOp("clk", OpAction.Write, 1, 3)

        #write rx={1} -> rx{addr} -> rx{data}
        def uartWriteOps(start, count):
            frameCycles = UART_BAUD_PERIOD * (3 + UART_DATA_WIDTH)
            # HDL seems to expect stop bit for two cycles
            # so data width + start bit + 2 stop bits
            opCycles = frameCycles * 3
            addr = 0
            wFlagFrame = ((0b11 << UART_DATA_WIDTH) + 1) << 1 # data is LSB of 1 for writes 
            data = 1
            for i in range(UART_DATA_WIDTH-1):
                data = (data << 1) + 1
            wDataFrame = (0b11 << UART_DATA_WIDTH) #shift the stop bits (always 1)
            wDataFrame += data                  # add in the actual data (all 1s)
            wDataFrame = (wDataFrame << 1)      #shift for the start bit (always 0)
            for j in range(count):
                wAddrFrame = (0b11 << UART_ADDR_WIDTH) #shift the stop bits (always 1)
                wAddrFrame += addr                  # add in the actual data (all 1s)
                wAddrFrame = (wAddrFrame << 1)      #shift for the start bit (always 0)
                opStart = start + opCycles * j
                frames = [ wFlagFrame, wAddrFrame, wDataFrame ]
                for frame in frames:
                    bit = 0
                    for i in range(opStart, opStart + frameCycles):
                        self.addTestOp("clk", OpAction.Write, 0, i)
                        if (i % UART_BAUD_PERIOD == 0):
                            # send frame bit
                            self.addTestOp("RX", OpAction.Write, (frame >> bit)&1, i)
                            bit += 1
                        self.addTestOp("clk", OpAction.Write, 1, i)
                    opStart += frameCycles
                addr += 1
                addr = addr % (1 << UART_ADDR_WIDTH)
            return opStart # returns num of next cycle

        #read rx={0} -> rx{addr} -> tx{data}
        def uartReadOps(start, count):
            frameCycles = UART_BAUD_PERIOD * (3 + UART_DATA_WIDTH)
            # HDL seems to expect stop bit for two cycles
            # so data width + start bit + 2 stop bits
            opCycles = frameCycles * 3
            addr = 0
            rFlagFrame = 0b11 << (UART_DATA_WIDTH + 1) # data is LSB of 0 for reads 
            data = 1
            for i in range(UART_DATA_WIDTH-1):
                data = (data << 1) + 1
            rDataFrame = (0b11 << UART_DATA_WIDTH) #shift the stop bit (always 1)
            rDataFrame += data                  # add in the actual data (all 1s)
            rDataFrame = (rDataFrame << 1)      #shift for the start bit (always 0)
            for j in range(count):
                rAddrFrame = (0b11 << UART_ADDR_WIDTH) #shift the stop bit (always 1)
                rAddrFrame += addr                  # add in the actual data (all 1s)
                rAddrFrame = (rAddrFrame << 1)      #shift for the start bit (always 0)
                opStart = start + opCycles * j
                # only transmit two frames (flag, address)
                frames = [ rFlagFrame, rAddrFrame ]
                for frame in frames:
                    # do the writing for flag/address frames bit by bit
                    for i in range(opStart, opStart + frameCycles):
                        self.addTestOp("clk", OpAction.Write, 0, i)
                        if (i % UART_BAUD_PERIOD == 0):
                            # send frame bit
                            bit = (i - opStart) // UART_BAUD_PERIOD
                            self.addTestOp("RX", OpAction.Write, (frame >> bit)&1, i)
                        self.addTestOp("clk", OpAction.Write, 1, i)
                    opStart += frameCycles
                # now make sure the incoming frame is correct bit by bit
                for i in range(opStart, opStart + frameCycles):
                    self.addTestOp("clk", OpAction.Write, 0, i)
                    if (i % UART_BAUD_PERIOD == UART_BAUD_PERIOD-1):
                        # send frame bit
                        bit = (i - opStart) // UART_BAUD_PERIOD
                        self.addTestOp("TX", OpAction.Read, (rDataFrame >> bit)&1, i)
                    self.addTestOp("clk", OpAction.Write, 1, i)
                addr += 1
                addr = addr % (1 << UART_ADDR_WIDTH)

        initUart()
        # count of read ops must not be higher than write ops 
        nextCycle = uartWriteOps(4, 4)
        uartReadOps(nextCycle, 4)

    def buildPicoTest(self, numCycles):

        def instrFetchCheck(addr, instr, cycle):
            self.addTestOp("mem_valid", OpAction.Read, 1, cycle)
            self.addTestOp("mem_instr", OpAction.Read, 1, cycle)
            self.addTestOp("mem_addr", OpAction.Read, addr, cycle)
            self.addTestOp("mem_ready", OpAction.Write, 1, cycle)
            self.addTestOp("mem_rdata", OpAction.Write, instr, cycle)

        def writeOpCheck(addr, val, cycle):
            self.addTestOp("mem_valid", OpAction.Read, 1, cycle)
            self.addTestOp("mem_instr", OpAction.Read, 0, cycle)
            self.addTestOp("mem_addr", OpAction.Read, addr, cycle)
            # check wstrb is f because the instructions we're sending
            # do a 4 byte store
            self.addTestOp("mem_wstrb", OpAction.Read, 0x000f, cycle)
            self.addTestOp("mem_ready", OpAction.Write, 1, cycle)
            self.addTestOp("mem_wdata", OpAction.Read, val, cycle)

        def readOpCheck(addr, val, cycle):
            self.addTestOp("mem_valid", OpAction.Read, 1, cycle)
            self.addTestOp("mem_instr", OpAction.Read, 0, cycle)
            self.addTestOp("mem_addr", OpAction.Read, addr, cycle)
            self.addTestOp("mem_ready", OpAction.Write, 1, cycle)
            self.addTestOp("mem_rdata", OpAction.Write, val, cycle)

        self.addTestOp("resetn", OpAction.Write, 1, 0)
        self.addTestOp("resetn", OpAction.Write, 0, 1)
        # Keep mem ready low until there's a transaction
        self.addTestOp("mem_ready", OpAction.Write, 0, 1)
        self.addTestOp("resetn", OpAction.Write, 1, 6)
        self.addTestOp("clk", OpAction.Write, 1, 6)
        self.addTestOp("clk", OpAction.Write, 0, 6)
        dataVal = 0
        # InstrMem emulates an instruction memory by holding the 6 instructions the firmware
        # should hold, so the test writes those instructions when an IF should be occurring
        InstrMem = [0x3fc00093, 0x0000a023, 0x0000a103, 0x00110113, 0x0020a023, 0xff5ff06f]
        # first check a few instructions worth of outputs
        # the firmware program is not yet looping
        for i in range(7, 22):
            self.addTestOp("clk", OpAction.Write, 1, i) # cycle clock every cycle
            if (i == 9):
                instrFetchCheck(0x0, InstrMem[0], i)
            if (i == 10):
                # Need to set mem_ready low now since the memory operation has ended
                # Same thing one cycle after every memory op
                self.addTestOp("mem_ready", OpAction.Write, 0, i)
            if (i == 13):
                instrFetchCheck(0x0004, InstrMem[1], i)
            if (i == 14):
                self.addTestOp("mem_ready", OpAction.Write, 0, i)
            if (i == 17):
                instrFetchCheck(0x0008, InstrMem[2], i)
            if (i == 18):
                self.addTestOp("mem_ready", OpAction.Write, 0, i)
            if (i == 20):
                # write operation to addr 3fc
                writeOpCheck(0x03fc, dataVal, i)
            if (i % 25 == 21):
                self.addTestOp("mem_ready", OpAction.Write, 0, i)
            self.addTestOp("clk", OpAction.Write, 0, i) # cycle clock every cycle
        # Now the firmware is in the loop so generate test ops
        # based on modulo, same ops will be happening in a cycle
        for i in range(22, numCycles):
            self.addTestOp("clk", OpAction.Write, 1, i) # cycle clock every cycle
            if (i % 22 == 2):
                instrFetchCheck(0x000c, InstrMem[3], i)
            if (i % 22 == 3):
                self.addTestOp("mem_ready", OpAction.Write, 0, i)
            if (i % 22 == 5):
                # read operation to address 0x03fc
                readOpCheck(0x03fc, dataVal, i)
                dataVal += 1 # increment data value to match the firmware
            if (i % 22 == 6):
                self.addTestOp("mem_ready", OpAction.Write, 0, i)
            if (i % 22 == 9):
                instrFetchCheck(0x0010, InstrMem[4], i)
            if (i % 22 == 10):
                self.addTestOp("mem_ready", OpAction.Write, 0, i)
            if (i % 22 == 13):
                instrFetchCheck(0x0014, InstrMem[5], i)
            if (i % 22 == 14):
                self.addTestOp("mem_ready", OpAction.Write, 0, i)
            if (i % 22 == 16):
                # write operation to addr 3fc
                writeOpCheck(0x03fc, dataVal, i)
            if (i % 22 == 17):
                self.addTestOp("mem_ready", OpAction.Write, 0, i)
            if (i % 22 == 20):
                instrFetchCheck(0x0008, InstrMem[2], i)
            if (i % 22 == 21):
                self.addTestOp("mem_ready", OpAction.Write, 0, i)
            self.addTestOp("clk", OpAction.Write, 0, i) # cycle clock every cycle

          

    def getTest(self):
        return(self.TestOps)

    def __str__(self):
        return(str(self.TestOps))

    def export(self):
        for op in self.TestOps:
            print(op)


def run_direct(subName, verbosity, verbosityMask, vpi, numCycles):
    testScheme = Test()
    # tell Test to ignore clk writes
    testScheme.setDirectMode()
    if ( subName == "Counter" ):
        testScheme.buildCounterTest(numCycles, 1)
        print("Basic test for Counter:")
    elif ( subName == "Accum" ):
        testScheme.buildAccumTest(numCycles)
        print("Basic test for Accum:")
    elif ( subName == "Accum1D" ):
        testScheme.buildAccum1DTest(numCycles)
        print("Basic test for Accum1D:")
    elif ( subName == "UART" ):
        testScheme.buildUartTest(numCycles)
        print("Basic test for UART:")
    elif ( subName == "Scratchpad" ):
        testScheme.buildScratchTest(numCycles)
        print("Basic test for Scratchpad:")
    elif ( subName == "Pin" ):
        testScheme.buildPinTest(numCycles)
        print("Basic test for Pin:")
    elif ( subName == "PicoRV" ):
        testScheme.buildPicoTest(numCycles)
        print("Basic test for PicoRV:")

    print(testScheme)
    top = sst.Component("top0", "verilatortestdirect.VerilatorTestDirect")
    top.addParams({
        "verbose" : verbosity,
        "verboseMask" : verbosityMask,
        "clockFreq" : "1GHz",
        "testOps" : testScheme.getTest(),
        "numCycles" : numCycles
    })
    print(f"Running direct test for {subName}Direct")
    fullName = f"verilatorsst{subName}Direct.VerilatorSST{subName}"
    model = top.setSubComponent("model", f"{fullName}Direct")
    model.addParams({
        "useVPI" : vpi,
        "clockFreq" : "1GHz",
        "clockPort" : "clk",
    })

def run_links(subName, verbosity, verbosityMask, vpi, numCycles):
    testScheme = Test()
    ports = PortDef()
    if ( subName == "Counter" ):
        ports.addPort("clk",     1, WRITE_PORT)
        ports.addPort("reset_l", 1, WRITE_PORT)
        ports.addPort("stop",    1, WRITE_PORT)
        ports.addPort("done",    1, READ_PORT)
        testScheme.buildCounterTest(numCycles, 0)
        print(ports.getPortMap())
        print("Basic test for Counter:")
    elif ( subName == "Accum" ):
        ports.addPort("clk",     1,  WRITE_PORT)
        ports.addPort("reset_l", 1,  WRITE_PORT)
        ports.addPort("en",      1,  WRITE_PORT)
        ports.addPort("add",     8,  WRITE_PORT)
        ports.addPort("accum",   16, READ_PORT)
        ports.addPort("done",    1,  READ_PORT)
        testScheme.buildAccumTest(numCycles)
        print(ports.getPortMap())
        print("Basic test for Accum:")
    elif ( subName == "Accum1D" ):
        ports.addPort("clk",     1,  WRITE_PORT)
        ports.addPort("reset_l", 1,  WRITE_PORT)
        ports.addPort("en",      1,  WRITE_PORT)
        ports.addPort("add",     16, WRITE_PORT)
        ports.addPort("accum",   32, READ_PORT)
        ports.addPort("done",    1,  READ_PORT)
        testScheme.buildAccum1DTest(numCycles)
        print(ports.getPortMap())
        print("Basic test for Accum1D:")
    elif ( subName == "UART" ):
        ports.addPort("clk",       1,  WRITE_PORT)
        ports.addPort("rst_l",     1,  WRITE_PORT)
        ports.addPort("RX",        1,  WRITE_PORT)
        ports.addPort("TX",        1,  READ_PORT)
        # NOTE: mem_debug port is unused for testing
        ports.addPort("mem_debug", 1,  READ_PORT)
        testScheme.buildUartTest(numCycles)
        print(ports.getPortMap())
        print("Basic test for UART:")
    elif ( subName == "Scratchpad" ):
        ports.addPort("clk",   1, WRITE_PORT)
        ports.addPort("en",    1, WRITE_PORT)
        ports.addPort("write", 1, WRITE_PORT)
        ports.addPort("addr",  8, WRITE_PORT)
        ports.addPort("len",   1, WRITE_PORT)
        ports.addPort("wdata", 8, WRITE_PORT)
        ports.addPort("rdata", 8, READ_PORT)
        testScheme.buildScratchTest(numCycles)
        print(ports.getPortMap())
        print("Basic test for Scratchpad:")
    elif ( subName == "Pin" ):
        ports.addPort("direction",  1,  WRITE_PORT)
        ports.addPort("data_write", 1,  WRITE_PORT)
        ports.addPort("data_read",  1,  READ_PORT)
        ports.addPort("io_port",    1,  INOUT_PORT)
        ports.addPort("clk",        1,  WRITE_PORT)
        testScheme.buildPinTest(numCycles)
        print(ports.getPortMap())
        print("Basic test for Pin:")
    elif ( subName == "PicoRV" ):
        ports.addPort("clk", 1, WRITE_PORT)
        ports.addPort("resetn", 1, WRITE_PORT)
        ports.addPort("mem_ready", 1, WRITE_PORT)
        ports.addPort("pcpi_wr", 1, WRITE_PORT)
        ports.addPort("pcpi_wait", 1, WRITE_PORT)
        ports.addPort("pcpi_ready", 1, WRITE_PORT)
        ports.addPort("mem_rdata", 4, WRITE_PORT)
        ports.addPort("pcpi_rd", 4, WRITE_PORT)
        ports.addPort("irq", 4, WRITE_PORT)
        ports.addPort("trap", 1, READ_PORT)
        ports.addPort("mem_valid", 1, READ_PORT)
        ports.addPort("mem_instr", 1, READ_PORT)
        ports.addPort("mem_wstrb", 1, READ_PORT)
        ports.addPort("mem_la_read", 1, READ_PORT)
        ports.addPort("mem_la_write", 1, READ_PORT)
        ports.addPort("mem_la_wstrb", 1, READ_PORT)
        ports.addPort("pcpi_valid", 1, READ_PORT)
        ports.addPort("trace_valid", 1, READ_PORT)
        ports.addPort("mem_addr", 4, READ_PORT)
        ports.addPort("mem_wdata", 4, READ_PORT)
        ports.addPort("mem_la_addr", 4, READ_PORT)
        ports.addPort("mem_la_wdata", 4, READ_PORT)
        ports.addPort("pcpi_insn", 4, READ_PORT)
        ports.addPort("pcpi_rs1", 4, READ_PORT)
        ports.addPort("pcpi_rs2", 4, READ_PORT)
        ports.addPort("eoi", 4, READ_PORT)
        ports.addPort("trace_data", 5, READ_PORT)
        testScheme.buildPicoTest(numCycles)
        #testScheme.export()
        print(ports.getPortMap())
        print("Basic test for PicoRV:")
    print(testScheme)

    tester = sst.Component("vtestLink0", "verilatortestlink.VerilatorTestLink")
    tester.addParams({
        "verbose" : verbosity,
        "verboseMask" : verbosityMask,
        "clockFreq" : "1GHz",
        "num_ports" : ports.getNumPorts(),
        "portMap" : ports.getPortMap(),
        #"testFile" : "pico_test_ops.txt",
        "testOps" : testScheme.getTest(),
        "numCycles" : numCycles
    })

    # VerilatorComponent just holds the subcomponent
    verilatorsst = sst.Component("vsst", "verilatorcomponent.VerilatorComponent")
    verilatorsst.addParams({
        "numCycles" : numCycles
    })
    subCompName  = f"verilatorsst{subName}.VerilatorSST{subName}"
    # subcomponent contains the actual verilated module
    model = verilatorsst.setSubComponent("model", subCompName)
    model.addParams({
        "useVPI" : vpi,
        "clockFreq" : "2.0GHz",
        "clockPort" : "clk"
    })

    Links = [ ]
    # connect each verilator subcomponent port with a VerilatorTestLink port
    for i in range(ports.getNumPorts()):
        Links.append( sst.Link( f"link{i}" ) )
        Links[i].connect( ( model, ports.getPortName( i ), "0ps" ), ( tester, f"port{i}", "0ps" ) )

def main():

    examples = ["Counter", "Accum", "Accum1D", "UART", "Scratchpad", "Pin", "PicoRV"]
    parser = argparse.ArgumentParser(description="Sample script to run verilator SST examples")
    parser.add_argument("-m", "--model", choices=examples, default="Accum", help=("Select model from examples: "+str(examples)))
    parser.add_argument("-i", "--interface", choices=["links", "direct"], default="links", help="Select the direct testing method or the SST::Link method")
    parser.add_argument("-v", "--verbose", choices=range(15), default=4, help="Set the level of verbosity used by the test components")
    parser.add_argument("-a", "--access", choices=["vpi", "direct"], default="direct", help="Select the method used by the subcomponent to read/write the verilated model's ports")
    parser.add_argument("-k", "--mask", choices=[choice.name for choice in VerboseMasking], default="FULL")
    parser.add_argument("-c", "--cycles", default=50, help="Set number of cycles the simulation will run for")
    parser.add_argument("-t", "--testfile", default="", help="Pass test ops from an external text file instead of from Python script")

    args = parser.parse_args()

    if args.model not in examples:
        raise Exception("Unknown model selected")

    sub = args.model
    numCycles = int(args.cycles)
    chosenMask = args.mask
    verbosityMask = VerboseMasking[chosenMask].value
    print("Using verbosityMask {}".format(verbosityMask))
    verbosity = args.verbose
    if (args.access == "vpi"):
        vpi = 1
    else:
        vpi = 0


    if args.interface == "direct":
        run_direct(sub, verbosity, verbosityMask, vpi, numCycles)
    elif args.interface == "links":
        run_links(sub, verbosity, verbosityMask, vpi, numCycles)
          
    sst.setStatisticLoadLevel(7)
    sst.setStatisticOutput("sst.statOutputCSV")
    sst.enableAllStatisticsForAllComponents()

if __name__ == "__main__":
    main()
