#
# basicVerilogCounter.py
#

import verilatorSST

verilatorSST.create_component("CounterComponent", "basicverilogcounter.BasicVerilogCounter")
verilatorSST.define_macros("CounterComponent", [('INPUT_WIDTH',16),('STOP',15),('CLOCK_FREQ','1Ghz')])
verilatorSST.finalize("CounterComponent")

## equivalent to ....

# clockComponent = sst.Component("CounterComponent", "basicverilogcounter.BasicVerilogCounter")
# clockComponent.addParams({
#   "INPUT_WIDTH" : 16,
#   "STOP"  : "15",
#   "CLOCK_FREQ"  : "1Ghz",
# }) 
