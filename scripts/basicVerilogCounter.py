import verilatorSST

verilatorSST.create_component("CounterComponent", "BasicVerilogCounter")
verilatorSST.define_macros("CounterComponent", [('STOP_WIDTH',8),('STOP',15),('CLOCK_FREQ','1Ghz')])
verilatorSST.finalize("CounterComponent")
