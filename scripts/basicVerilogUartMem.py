import verilatorSST

verilatorSST.create_component("UartComponent", "BasicVerilogUartMem")
verilatorSST.define_macros("UartComponent", [('ADDR_WIDTH',9),('DATA_WIDTH',9),('BAUD_PERIOD',16),('CLOCK_FREQ','1Ghz')])
verilatorSST.finalize("UartComponent")
