import sst

accum_component = sst.Component("AccumComponent", "verilatorsst.BigVerilogAccum")
accum_component.addParams({
    'ACCUM_WIDTH': 88,
    'ADD':         0x1010406080a0c0e1,
    'ADD2':        0xf311517191b1d1f1,
    'CLOCK_FREQ': '1Ghz'
})
