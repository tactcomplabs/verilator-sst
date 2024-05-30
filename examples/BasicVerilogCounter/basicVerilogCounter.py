import sst

counter_component = sst.Component("CounterComponent", "verilatorsst.BasicVerilogCounter")
counter_component.addParams({
    'STOP_WIDTH':8
})
