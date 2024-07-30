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

examples = ["Counter", "Accum", "UART", "Scratchpad"]
if (len(sys.argv) > 2 and sys.argv[1] == "-m"):
     sub = sys.argv[2]
     if sub not in examples:
          raise Exception("Unknown model selected")
     subName = "verilatorsst{}.VerilatorSST{}".format(sub, sub)
else:
     # Default to accum because it is a relatively robust example
     subName = "verilatorsstAccum.VerilatorSSTAccum"

top = sst.Component("top0", "verilatortestdirect.VerilatorTestDirect")
top.addParams({
  "verbose" : 1,
  "clockFreq" : "1GHz",
  "numCycles" : 5000
})
model = top.setSubComponent("model", subName)
model.addParams({
  "useVPI" : 1,
  "clockFreq" : "1GHz",
  "clockPort" : "clk",
  #"resetVals" : ["reset_l:0", "clk:0", "add:16", "en:0"]
})
