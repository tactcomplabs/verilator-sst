import sst

uart_component = sst.Component("UartMemComponent", "verilatorsst.BasicVerilogUartMem")
uart_component.addParams({
    'ADDR_WIDTH': 8,
    'DATA_WIDTH': 16,
    'FRAME_WIDTH': 16,
    'BAUD_PERIOD':16,
    'CLOCK_FREQ':'1Ghz'
})
