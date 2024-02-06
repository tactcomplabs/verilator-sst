#
# basicVerilogCounter.py
#

import verilatorSST
import sst

verilatorSST.add_component("CounterComponent", "basicComponent.basicVerilogCounter")
verilatorSST.define_macro("CounterComponent", [('MAX',5),('clockFreq','1Ghz')])
verilatorSST.finalize("CounterComponent")

## equivalent to ....

# clockComponent = sst.Component("CounterComponent", "basicComponent.basicVerilogCounter")
# clockComponent.addParams({
#   "maxCount" : 4,
#   "clockFreq"  : "1Ghz",
# }) 
