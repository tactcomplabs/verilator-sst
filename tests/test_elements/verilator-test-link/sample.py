#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#
# sample.py
#

import os
import sys
import queue
import random
import sst

WRITE_PORT = "1"
READ_PORT = "0"
UINT64_MAX = 0xffff_ffff_ffff_ffff

examples = ["Counter", "Accum", "Accum1D", "UART", "Scratchpad"]
if (len(sys.argv) > 2 and sys.argv[1] == "-m"):
     sub = sys.argv[2]
     if sub not in examples:
          raise Exception("Unknown model selected")
     subName = "verilatorsst{}.VerilatorSST{}".format(sub, sub)
else:
     raise Exception("Unknown model selected")

class PortDef:
     """ Wrapper class to make port definitions cleaner """
     def __init__(self):
          self.PortList = [ ]
          self.PortId = 0
          self.PortNames = [ ]

     # portName is a string, portSize is an int (measured in bytes), portDir
     # should use READ_PORT or WRITE_PORT globals
     def addPort(self, portName, portSize, portDir):
          tmp = portName + ":" + str(self.PortId) + ":" + str(portSize) + ":" + portDir 
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
          global scratchAddrBase
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
                    randAddr = randIntBySize(8) # address is always 64 bit
                    self.randValues.put(randAddr)
                    self.addTestOp("addr", scratchAddrBase + randAddr, i)
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
                    self.addTestOp("addr", scratchAddrBase + randAddr, i)
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
          for i in range(4, numCycles):
               self.addTestOp("clk", 1, i) # cycle clock every cycle
               if (i % 3 == 1):
                    randDataLower = randIntBySize(8)
                    randDataUpper = randIntBySize(8)
                    #bigAdd = [randDataLower, randDataUpper]
                    bigAdd = [0, 0]
                    #accum[0] += randDataLower
                    #accum[1] += randDataUpper
                    for j in range(4):
                         ele = accum[j]
                         # python can have ints bigger than 64bit but the C++ side can't
                         # carry over, if a "64bit" integer "overflowed"
                         if ele > UINT64_MAX and j < 3:
                              diff = ele - UINT64_MAX
                              accum[j] = UINT64_MAX
                              accum[j+1] += diff
                    self.addBigTestOp("add", bigAdd, i)
                    self.addTestOp("en", 1, i)
               elif (i % 3 == 2):
                    self.addBigTestOp("accum", accum, i)
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

scratchAddrBase = 0x0300_0000_0000_0000
numCycles = 50 
testScheme = Test()
ports = PortDef()
if ( sub == "Counter" ):
     ports.addPort("clk",     1, WRITE_PORT)
     ports.addPort("reset_l", 1, WRITE_PORT)
     ports.addPort("stop",    1, WRITE_PORT)
     ports.addPort("done",    1, READ_PORT)
     testScheme.buildCounterTest(numCycles)
     print(ports.getPortMap())
     print("Basic test for Counter:")
elif ( sub == "Accum" ):
     ports.addPort("clk",     1,  WRITE_PORT)
     ports.addPort("reset_l", 1,  WRITE_PORT)
     ports.addPort("en",      1,  WRITE_PORT)
     ports.addPort("add",     8,  WRITE_PORT)
     ports.addPort("accum",   16, READ_PORT)
     ports.addPort("done",    1,  READ_PORT)
     print(ports.getPortMap())
elif ( sub == "Accum1D" ):
     ports.addPort("clk",     1,  WRITE_PORT)
     ports.addPort("reset_l", 1,  WRITE_PORT)
     ports.addPort("en",      1,  WRITE_PORT)
     ports.addPort("add",     16, WRITE_PORT)
     ports.addPort("accum",   32, READ_PORT)
     ports.addPort("done",    1,  READ_PORT)
     testScheme.buildAccum1DTest(numCycles)
     print(ports.getPortMap())
     print("Basic test for Accum1D:")
elif ( sub == "UART" ):
     ports.addPort("clk",       1,  WRITE_PORT)
     ports.addPort("rst_l",     1,  WRITE_PORT)
     ports.addPort("RX",        1,  WRITE_PORT)
     ports.addPort("TX",        1,  READ_PORT)
     ports.addPort("mem_debug", 1,  READ_PORT)
elif ( sub == "Scratchpad" ):
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
  "verbose" : 4,
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
model = verilatorsst.setSubComponent("model", subName) # and verilatorsst subcomponent
model.addParams({
    "useVPI" : 0,
    "clockFreq" : "1.0GHz",
    "clockPort" : "clk"
})

Links = [ ]
for i in range(ports.getNumPorts()):
     Links.append( sst.Link( "link"+str(i) ) )
     Links[i].connect( ( model, ports.getPortName( i ), "0ps" ), ( tester, "port"+str(i), "0ps" ) )
