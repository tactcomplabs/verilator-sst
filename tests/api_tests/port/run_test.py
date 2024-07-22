# tests/integration_tests/port/run_test.py
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details

import sst

testBench = sst.Component("testBench", "verilatorssttests.PortTestBench")
testBench.addParams({
    "verbose" : 1,
    "clockFreq" : "1GHz",
    "numCycles" : 5000
})

dut = testBench.setSubComponent("dut", "verilatorsstDUT.VerilatorSSTDUT")
dut.addParams({
    "clockFreq" : "1GHz"
})
