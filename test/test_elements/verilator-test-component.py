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

class OpAction(Enum):
     Write = "write"
     Read  = "read"

class VerboseMasking(IntEnum):
     WRITE_EVENT  = 0b1
     READ_EVENT   = 0b10
     ALL_EVENTS   = 0b11
     WRITE_PORT   = 0b100
     READ_PORT    = 0b1000
     INOUT_PORT   = 0b10000
     ALL_PORTS    = 0b11100
     READ_DATA    = 0b100000
     WRITE_DATA   = 0b1000000
     ALL_DATA     = 0b1100000
     TEST_OP      = 0b10000000
     WRITE_QUEUE  = 0b100000000
     INIT         = 0b1000000000
     FULL         = 0b1111111111

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
          # direct mode ignores clk writes

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

     def getTest(self):
          return(self.TestOps)

     def printTest(self):
          print(self.TestOps)

def run_direct(subName, verbosity, verbosityMask, vpi):
    numCycles = 50
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
         print("No test for UART yet. . . ")
         # nothing yet
    elif ( subName == "Scratchpad" ):
         testScheme.buildScratchTest(numCycles)
         print("Basic test for Scratchpad:")
    elif ( subName == "Pin" ):
         testScheme.buildPinTest(numCycles)
         print("Basic test for Pin:")

    testScheme.printTest()
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
        #"resetVals" : ["reset_l:0", "clk:0", "add:16", "en:0"]
    })

def run_links(subName, verbosity, verbosityMask, vpi):
    numCycles = 50
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
         ports.addPort("mem_debug", 1,  READ_PORT)
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
         ports.addPort("clk",        1,  WRITE_PORT) #TODO order here determines order of link events
         testScheme.buildPinTest(numCycles)
         print(ports.getPortMap())
         print("Basic test for Pin:")
    testScheme.printTest()

    tester = sst.Component("vtestLink0", "verilatortestlink.VerilatorTestLink")
    tester.addParams({
        "verbose" : verbosity,
        "verboseMask" : verbosityMask,
        "clockFreq" : "1GHz",
        "num_ports" : ports.getNumPorts(),
        "portMap" : ports.getPortMap(),
        "testOps" : testScheme.getTest(),
        "numCycles" : numCycles
    })

    verilatorsst = sst.Component("vsst", "verilatorcomponent.VerilatorComponent") # and verilatorsst component
    verilatorsst.addParams({
        "numCycles" : numCycles
    })
    subCompName  = f"verilatorsst{subName}.VerilatorSST{subName}"
    model = verilatorsst.setSubComponent("model", subCompName) # and verilatorsst subcomponent
    model.addParams({
        "useVPI" : vpi,
        "clockFreq" : "2.0GHz",
        "clockPort" : "clk"
    })

    Links = [ ]
    for i in range(ports.getNumPorts()):
        Links.append( sst.Link( f"link{i}" ) )
        Links[i].connect( ( model, ports.getPortName( i ), "0ps" ), ( tester, f"port{i}", "0ps" ) )

def main():

    examples = ["Counter", "Accum", "Accum1D", "UART", "Scratchpad", "Pin"]
    parser = argparse.ArgumentParser(description="Sample script to run verilator SST examples")
    parser.add_argument("-m", "--model", choices=examples, default="Accum", help="Select model from examples: Counter, Accum(1D), UART, Scratchpad, Pin")
    parser.add_argument("-i", "--interface", choices=["links", "direct"], default="links", help="Select the direct testing method or the SST::Link method")
    parser.add_argument("-v", "--verbose", choices=range(15), default=4, help="Set the level of verbosity used by the test components")
    parser.add_argument("-a", "--access", choices=["vpi", "direct"], default="direct", help="Select the method used by the subcomponent to read/write the verilated model's ports")
    parser.add_argument("-k", "--mask", choices=[choice.name for choice in VerboseMasking], default="FULL")

    args = parser.parse_args()

    if args.model not in examples:
        raise Exception("Unknown model selected")

    sub = args.model
    chosenMask = args.mask
    verbosityMask = VerboseMasking[chosenMask].value
    print("Using verbosityMask {}".format(verbosityMask))
    verbosity = args.verbose
    if (args.access == "vpi"):
        vpi = 1
    else:
        vpi = 0


    if args.interface == "direct":
        run_direct(sub, verbosity, verbosityMask, vpi)
    elif args.interface == "links":
        run_links(sub, verbosity, verbosityMask, vpi)

if __name__ == "__main__":
    main()
