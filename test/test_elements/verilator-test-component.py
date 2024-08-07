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

WRITE_PORT = "1"
READ_PORT = "0"
UINT64_MAX = 0xffff_ffff_ffff_ffff
SCRATCH_ADDR_BASE = 0x0300_0000_0000_0000
SCRATCH_SIZE = 512 * 1024

class PortDef:
     """ Wrapper class to make port definitions cleaner """
     def __init__(self):
          self.PortList = [ ]
          self.PortId = 0
          self.PortNames = [ ]

     # portName is a string, portSize is an int (measured in bytes), portDir
     # should use READ_PORT or WRITE_PORT globals
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

     # Used for test ops that have a value <=64bit
     def addTestOp(self, portName, value, tick):
          tmp = portName + ":" + str(value) + ":" + str(tick)
          self.TestOps.append(tmp)

     # Used for test ops that have values >64bit, requires a list of 64 bit values
     def addBigTestOp(self, portName, values, tick):
          tmp = portName + ":"
          for val in values:
               tmp += str(val) + ":"
          tmp += str(tick)
          self.TestOps.append(tmp)

     def buildScratchTest(self, numCycles):
          global SCRATCH_ADDR_BASE
          global SCRATCH_SIZE
          for i in range(numCycles):
               self.addTestOp("clk", 1, i) # cycle clock every cycle
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
                    self.addTestOp("write", 1, i)
                    randAddr = randIntBySize(8) % SCRATCH_SIZE # address is always 64 bit
                    self.randValues.put(randAddr)
                    self.addTestOp("addr", SCRATCH_ADDR_BASE + randAddr, i)
                    self.addTestOp("len", length, i)
                    randData = randIntBySize(sizeToWrite)
                    self.randValues.put(randData)
                    self.addTestOp("wdata", randData, i)
                    self.addTestOp("en", 1, i)
               elif (i % 5 == 2):
                    self.addTestOp("en", 0, i)
               elif (i % 5 == 3):
                    self.addTestOp("write", 0, i)
                    randAddr = self.randValues.get()
                    self.addTestOp("addr", SCRATCH_ADDR_BASE + randAddr, i)
                    self.addTestOp("len", length, i)
                    self.addTestOp("en", 1, i)
               elif (i % 5 == 4):
                    randData = self.randValues.get()
                    self.addTestOp("rdata", randData, i)
                    self.addTestOp("en", 0, i)
               self.addTestOp("clk", 0, i)

     def buildAccum1DTest(self, numCycles):
          global UINT64_MAX
          self.addTestOp("reset_l", 0, 1)
          self.addTestOp("reset_l", 1, 3)
          self.addTestOp("clk", 1, 3)
          self.addTestOp("clk", 0, 3)
          accum = [0, 0, 0, 0]
          bigAccum = 0
          for i in range(4, numCycles):
               self.addTestOp("clk", 1, i) # cycle clock every cycle
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
                    self.addBigTestOp("add", bigAdd, i)
                    self.addTestOp("en", 1, i)
               elif (i % 3 == 2):
                    self.addBigTestOp("accum", accum, i)
                    self.addTestOp("done", 1, i)
                    self.addTestOp("en", 0, i)
               self.addTestOp("clk", 0, i) # cycle clock every cycle
     
     def buildAccumTest(self, numCycles):
          global UINT64_MAX
          self.addTestOp("reset_l", 0, 1)
          self.addTestOp("reset_l", 1, 3)
          self.addTestOp("clk", 1, 3)
          self.addTestOp("clk", 0, 3)
          accum = [0, 0, 0, 0]  # 32 bit each
          bigAccum = [0, 0] # 64 bit each
          add = [0, 0, 0, 0] # 16 bit each
          bigAdd = 0 # 64 bits
          for i in range(4, numCycles):
               self.addTestOp("clk", 1, i) # cycle clock every cycle
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
                    self.addTestOp("add", bigAdd, i)
                    self.addTestOp("en", 1, i)
               elif (i % 3 == 2):
                    self.addBigTestOp("accum", bigAccum, i)
                    self.addTestOp("done", 1, i)
                    self.addTestOp("en", 0, i)
               self.addTestOp("clk", 0, i) # cycle clock every cycle
     
     def buildCounterTest(self, numCycles):
          self.addTestOp("reset_l", 0, 1)
          self.addTestOp("reset_l", 1, 3)
          stopCycle = 4
          currStopVal = 0
          self.addTestOp("stop", currStopVal, 3)
          self.addTestOp("clk", 1, 3)
          self.addTestOp("clk", 0, 3)
          for i in range(4, numCycles):
               self.addTestOp("clk", 1, i) # cycle clock every cycle
               if (i < 11):
                    currStopVal += 1
                    self.addTestOp("done", 1, i)
                    self.addTestOp("stop", currStopVal, i)
                    stopCycle = i + 8
               elif (i == stopCycle):
                    stopCycle = i + 8
                    self.addTestOp("done", 1, i)
               else:
                    self.addTestOp("done", 0, i)
               self.addTestOp("clk", 0, i) # cycle clock every cycle



     def getTest(self):
          return(self.TestOps)

     def printTest(self):
          print(self.TestOps)

def run_direct(subName, verbosity, vpi):
    numCycles = 50
    testScheme = Test()
    if ( subName == "Counter" ):
         testScheme.buildCounterTest(numCycles)
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

    testScheme.printTest()
    top = sst.Component("top0", "verilatortestdirect.VerilatorTestDirect")
    top.addParams({
        "verbose" : verbosity,
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

def run_links(subName, verbosity, vpi):
    numCycles = 50
    testScheme = Test()
    ports = PortDef()
    if ( subName == "Counter" ):
         ports.addPort("clk",     1, WRITE_PORT)
         ports.addPort("reset_l", 1, WRITE_PORT)
         ports.addPort("stop",    1, WRITE_PORT)
         ports.addPort("done",    1, READ_PORT)
         testScheme.buildCounterTest(numCycles)
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

    testScheme.printTest()

    tester = sst.Component("vtestLink0", "verilatortestlink.VerilatorTestLink")
    tester.addParams({
        "verbose" : verbosity,
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

    examples = ["Counter", "Accum", "Accum1D", "UART", "Scratchpad"]
    parser = argparse.ArgumentParser(description="Sample script to run verilator SST examples")
    parser.add_argument("-m", "--model", choices=examples, default="Accum", help="Select model from examples: Counter, Accum(1D), UART, Scratchpad")
    parser.add_argument("-i", "--interface", choices=["links", "direct"], default="links", help="Select the direct testing method or the SST::Link method")
    parser.add_argument("-v", "--verbose", choices=range(15), default=4, help="Set the level of verbosity used by the test components")
    parser.add_argument("-a", "--access", choices=["vpi", "direct"], default="direct", help="Select the method used by the subcomponent to read/write the verilated model's ports")

    args = parser.parse_args()

    if args.model not in examples:
        raise Exception("Unknown model selected")

    sub = args.model
    verbosity = args.verbose
    if (args.access == "vpi"):
        vpi = 1
    else:
        vpi = 0


    if args.interface == "direct":
        subName = f"verilatorsst{args.model}Direct.VerilatorSST{args.model}"
        run_direct(sub, verbosity, vpi)
    elif args.interface == "links":
        #subName = f"verilatorsst{args.model}.VerilatorSST{args.model}"
        run_links(sub, verbosity, vpi)

if __name__ == "__main__":
    main()
