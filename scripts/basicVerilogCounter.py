import verilatorSST

verilatorSST.create_component("CounterComponent", "BasicVerilogCounter")
verilatorSST.define_macros("CounterComponent", [('INPUT_WIDTH',16),('STOP',15),('CLOCK_FREQ','1Ghz')])
verilatorSST.finalize("CounterComponent")
