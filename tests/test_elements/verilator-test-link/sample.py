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
import sst

WRITE_PORT = "1"
READ_PORT = "0"

examples = ["Counter", "Accum", "UART", "Scratchpad"]
if (len(sys.argv) > 2 and sys.argv[1] == "-m"):
     sub = sys.argv[2]
     if sub not in examples:
          raise Exception("Unknown model selected")
     subName = "verilatorsst{}.VerilatorSST{}".format(sub, sub)
else:
     # Default to accum because it is a relatively robust example
     sub = "Accum"
     subName = "verilatorsstAccum.VerilatorSSTAccum"

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

scratchAddrBase = 0x0300000000000000

class Test:
     """ Class for building and exporting tests """
     def __init__(self):
          self.TestOps = [ ]
     
     def addTestOp(self, portName, value, tick):
          tmp = portName + ":" + str(value) + ":" + str(tick)
          self.TestOps.append(tmp)
     
     def buildScratchTest(self, numCycles):
          global scratchAddrBase
          for i in range(numCycles):
               self.addTestOp("clk", 1, i)
               if (i % 7 == 1):
                    self.addTestOp("write", 1, i)
                    self.addTestOp("addr", scratchAddrBase + 4, i)
                    self.addTestOp("len", 2, i)
                    self.addTestOp("wdata", 44, i)
                    self.addTestOp("en", 1, i)
               elif (i % 7 == 2):
                    self.addTestOp("en", 0, i)
               elif (i % 7 == 3):
                    self.addTestOp("write", 0, i)
                    self.addTestOp("addr", scratchAddrBase + 4, i)
                    self.addTestOp("len", 2, i)
                    self.addTestOp("en", 1, i)
               elif (i % 7 == 4):
                    self.addTestOp("rdata", 44, i)
                    self.addTestOp("en", 0, i)
               self.addTestOp("clk", 0, i)
     
     def getTest(self):
          return(self.TestOps)
     
     def printTest(self):
          print(self.TestOps)

ports = PortDef()
if ( sub == "Counter" ):
     ports.addPort("clk",     1, WRITE_PORT)
     ports.addPort("reset_l", 1, WRITE_PORT)
     ports.addPort("stop",    1, WRITE_PORT)
     ports.addPort("done",    1, READ_PORT)
elif ( sub == "Accum" ):
     ports.addPort("clk",     1,  WRITE_PORT)
     ports.addPort("reset_l", 1,  WRITE_PORT)
     ports.addPort("en",      1,  WRITE_PORT)
     ports.addPort("add",     8,  WRITE_PORT)
     ports.addPort("accum",   16, READ_PORT)
     ports.addPort("done",    1,  READ_PORT)
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
     basicScratchTest = Test()
     basicScratchTest.buildScratchTest(10)
     testOps = basicScratchTest.getTest()
     print(ports.getPortMap())
     print("Basic test for Scratchpad:")
     print("Scratchpad base addr 0x0300.0000.0000.0000 = %d" % scratchAddrBase)
     basicScratchTest.printTest()

tester = sst.Component("vtestLink0", "verilatortestlink.VerilatorTestLink")
tester.addParams({
  "verbose" : 4,
  "clockFreq" : "1GHz",
  "num_ports" : ports.getNumPorts(),
  "portMap" : ports.getPortMap(),
  "testOps" : testOps,
  "numCycles" : 20
})

verilatorsst = sst.Component("vsst", "verilatorcomponent.VerilatorComponent") # and verilatorsst component
verilatorsst.addParams({
     "numCycles" : 20
})
model = verilatorsst.setSubComponent("model", "verilatorsstScratchpad.VerilatorSSTScratchpad") # and verilatorsst subcomponent
model.addParams({
    "useVPI" : 0,
    "clockFreq" : "2.0GHz",
    "clockPort" : "clk"
})

"""
model = tester.setSubComponent("model", subName)
model.addParams({
  "useVPI" : 1,
  "clockFreq" : "1GHz",
  "clockPort" : "clk",
})
"""

Links = [ ]
for i in range(ports.getNumPorts()):
     Links.append( sst.Link( "link"+str(i) ) )
     Links[i].connect( ( model, ports.getPortName( i ), "0ps" ), ( tester, "port"+str(i), "0ps" ) )
