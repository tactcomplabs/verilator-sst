import verilatorSST

verilatorSST.create_component("CounterComponent", "BasicVerilogUartMem")
verilatorSST.define_macros("CounterComponent", [('ADDR_WIDTH',4),('DATA_WIDTH',4),('BAUD_PERIOD',16),('CLOCK_FREQ','1Ghz')])
verilatorSST.finalize("CounterComponent")
