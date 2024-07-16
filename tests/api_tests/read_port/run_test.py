import sst

test_harness = sst.Component("testHarness", "verilatorssttests.TestHarness")
test_harness.addParams({
    "verbose" : 1,
    "clockFreq" : "1GHz",
    "numCycles" : 5000
})

dut = test_harness.setSubComponent("dut", "verilatorsstDUT.VerilatorSSTDUT")
dut.addParams({
    "clockFreq" : "1GHz"
})
