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
