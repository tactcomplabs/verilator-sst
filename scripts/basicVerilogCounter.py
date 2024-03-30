#
# basicVerilogCounter.py
#

import verilatorSST

verilatorSST.create_component("CounterComponent", "basicverilogcounter.BasicVerilogCounter")
verilatorSST.define_macros("CounterComponent", [('INPUT_WIDTH',16),('STOP',15),('CLOCK_FREQ','1Ghz')])
verilatorSST.finalize("CounterComponent")

## equivalent to ....

# clockComponent = sst.Component("CounterComponent", "basicComponent.basicVerilogCounter")
# clockComponent.addParams({
#   "maxCount" : 4,
#   "clockFreq"  : "1Ghz",
# }) 
