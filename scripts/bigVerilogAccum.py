import verilatorSST

verilatorSST.create_component("AccumComponent", "BigVerilogAccum")
#verilatorSST.define_macros("AccumComponent", [('ACCUM_WIDTH',128),('ADD',0x01030507090b0d04), ('ADD2', 0x01010406080a0c0e), ('CLOCK_FREQ','1Ghz')]) #full words example
verilatorSST.define_macros("AccumComponent", [('ACCUM_WIDTH',88),('ADD',0x1010406080a0c0e1), ('ADD2', 0xf311517191b1d1f1), ('CLOCK_FREQ','1Ghz')]) #partial word example
verilatorSST.finalize("AccumComponent")