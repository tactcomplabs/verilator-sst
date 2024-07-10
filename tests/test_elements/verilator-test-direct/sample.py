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

if (len(sys.argv) > 2 and sys.argv[1] == "-m"):
     test = sys.argv[2]
     file_path = os.path.abspath(__file__)
     test_path = os.path.join(file_path,
                              os.path.pardir,
                              os.path.pardir,
                              os.path.pardir,
                              test)
     test_abs_path = os.path.abspath(test_path)
     if not os.path.exists(test_abs_path):
          raise Exception("Unknown model selected")

     subName = "verilatorsst{}.VerilatorSST{}".format(test, test)
else:
     # Default to accum because it is a relatively robust example
     subName = "verilatorsstAccum.VerilatorSSTAccum"

top = sst.Component("top0", "verilatortestdirect.VerilatorTestDirect")
top.addParams({
  "verbose" : 1,
  "clockFreq" : "1GHz",
  "numCycles" : 5
})
model = top.setSubComponent("model", subName)
model.addParams({
  "useVPI" : 1,
  "clockFreq" : "1GHz",
  "clockPort" : "clk",
  #"resetVals" : ["reset_l:0", "clk:0", "add:16", "en:0"]
})
