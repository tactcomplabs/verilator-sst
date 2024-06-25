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
import sst

top = sst.Component("top0", "verilatortestdirect.VerilatorTestDirect")
top.addParams({
  "verbose" : 1,
  "clockFreq" : "1GHz",
  "numCycles" : 5000
})
"""
model = top.setSubComponent("model", "verilatorsstAccum.VerilatorSSTAccum")
model.addParams({
  "useVPI" : 1,
  "clockFreq" : "1GHz",
  "clockPort" : "clk",
})
model = top.setSubComponent("model", "verilatorsstUART.VerilatorSSTUART")
model.addParams({
  "useVPI" : 0,
  "clockFreq" : "1GHz",
  "clockPort" : "clk",
})
"""
model = top.setSubComponent("model", "verilatorsstCounter.VerilatorSSTCounter")
model.addParams({
  "useVPI" : 0,
  "clockFreq" : "1GHz",
  "clockPort" : "clk",
})
