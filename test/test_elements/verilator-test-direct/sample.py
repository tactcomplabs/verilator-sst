#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#
# sample.py
#

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
