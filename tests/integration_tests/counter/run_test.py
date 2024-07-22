# tests/integration_tests/counter/run_test.py
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details

import sst

testBench = sst.Component("testBench", "verilatorssttests.CounterTestBench")
testBench.addParams({
    "verbose" : 1,
    "clockFreq" : "1GHz",
    "numCycles" : 5000
})

dut = testBench.setSubComponent("dut", "verilatorsstCounter.VerilatorSSTCounter")
dut.addParams({
    "clockFreq" : "1GHz"
})
