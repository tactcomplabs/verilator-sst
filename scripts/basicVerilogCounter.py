#
# basicVerilogCounter.py
#

import verilatorSST

verilatorSST.add_component("CounterComponent", "basicverilogcounter.BasicVerilogCounter")
verilatorSST.define_macros("CounterComponent", [('_INPUT_WIDTH',8),('stop',15),('clockFreq','1Ghz')])
verilatorSST.finalize("CounterComponent")

## equivalent to ....

# clockComponent = sst.Component("CounterComponent", "basicComponent.basicVerilogCounter")
# clockComponent.addParams({
#   "maxCount" : 4,
#   "clockFreq"  : "1Ghz",
# }) 
