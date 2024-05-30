import sst

uart_component = sst.Component("CounterComponent", "verilatorsst.BasicVerilogUartMem")
uart_component.addParams({
    'ADDR_WIDTH':9,
    'DATA_WIDTH':9,
    'BAUD_PERIOD':16,
    'CLOCK_FREQ':'1Ghz'
})
