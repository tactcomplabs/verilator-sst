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
