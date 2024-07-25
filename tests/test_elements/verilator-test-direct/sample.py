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
import argparse

examples = ["Counter", "Accum", "UART", "Scratchpad"]

# Set up the argument parser
parser = argparse.ArgumentParser(description="Select a model for verilatorsst.")
parser.add_argument("-m", "--model", choices=examples, help="The model to use")

# Parse the arguments
args = parser.parse_args()

# Determine the subName based on the input
if args.model:
    sub = args.model
    subName = "verilatorsst{}.VerilatorSST{}".format(sub, sub)
else:
    # Default to accum because it is a relatively robust example
    subName = "verilatorsstAccum.VerilatorSSTAccum"

print(subName)
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
