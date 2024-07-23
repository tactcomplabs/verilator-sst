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


if ( sub == "Counter" ):
     numPorts = 4
elif ( sub == "Accum" ):
     numPorts = 6
elif ( sub == "UART" ):
     numPorts = 5
elif ( sub == "Scratchpad" ):
     numPorts = 7
     ports = PortDef()
     ports.addPort("clk",   1, WRITE_PORT)
     ports.addPort("en",    1, WRITE_PORT)
     ports.addPort("write", 1, WRITE_PORT)
     ports.addPort("addr",  8, WRITE_PORT)
     ports.addPort("len",   1, WRITE_PORT)
     ports.addPort("wdata", 8, WRITE_PORT)
     ports.addPort("rdata", 8, READ_PORT)
     testOps = ["clk:0:1", \
                    "en:1:2", "write:1:2", "addr:4:2", "len:2:2", "wdata:44:2", \
                    "clk:1:3",  \
                    "clk:0:4", "en:0:4", \
                    "clk:1:5", \
                    "clk:0:6", "en:1:6", "write:0:6", "addr:4:6", "len:2:6", "rdata:44:6", "clk:1:6",  \
                    "clk:0:7", "en:1:7", "clk:1:7","clk:0:7",\
                    "en:0:8", "rdata:44:8", "clk:1:8"]

tester = sst.Component("vtestLink0", "verilatortestlink.VerilatorTestLink")
tester.addParams({
  "verbose" : 4,
  "clockFreq" : "1GHz",
  "num_ports" : ports.getNumPorts(),
  "portMap" : ports.getPortMap(),
  "testOps" : testOps,
  "numCycles" : 5000
})

model = tester.setSubComponent("model", subName)
model.addParams({
  "useVPI" : 1,
  "clockFreq" : "1GHz",
  "clockPort" : "clk",
})

print(ports.getPortMap())

Links = [ ]
for i in range(ports.getNumPorts()):
     Links.append( sst.Link( "link"+str(i) ) )
     Links[i].connect( ( model, ports.getPortName( i ), "0ps" ), ( tester, "port"+str(i), "0ps" ) )
