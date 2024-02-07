#
# basicVerilogCounter.py
#

import verilatorSST
import sst

verilatorSST.add_component("CounterComponent", "basicComponent.basicVerilogCounter")
verilatorSST.define_macro("CounterComponent", [('INPUT_WIDTH',8),('START',5),('stop',15),('clockFreq','1Ghz')])
verilatorSST.finalize("CounterComponent")

## equivalent to ....

# clockComponent = sst.Component("CounterComponent", "basicComponent.basicVerilogCounter")
# clockComponent.addParams({
#   "maxCount" : 4,
#   "clockFreq"  : "1Ghz",
# }) 
