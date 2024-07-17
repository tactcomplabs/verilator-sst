#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#
# run_Counter.py
#

import sst

test_harness = sst.Component("top0", "verilatorssttestharness.TestHarness")
test_harness.addParams({
    "testSelect": 0,
    "verbose" : 1,
    "clockFreq" : "1GHz",
    "numCycles" : 5000
})

dut = test_harness.setSubComponent("dut", "Counter")
dut.addParams({
    "useVPI" : 1,
    "clockFreq" : "1GHz",
    "clockPort" : "clk"
})
