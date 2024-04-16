# VERILATOR-SST

### Dependencies

- Verilator v5.022 https://github.com/verilator/verilator/releases/tag/v5.022
- SST 13.0.0 https://github.com/sstsimulator/sst-core/releases/tag/v13.0.0_Final

## Build
```
git clone git@github.com:tactcomplabs/verilator-sst.git
cd verilator-sst/src
make
make verify
```

## Run
```
cd verilator-sst/scripts
sst basicVerilogCounter.py
```

## Debug
```
cd verilator-sst/scripts
DEBUG=1 sst basicVerilogCounter.py
```

## Dev notes

### Verilog memory order

The index order of Verilog vectors is dependent on how the index bounds are defined. They can be either ascending or descending order. Below, mem1 is defined in ascending order, and mem2 in descending. 

```
top.v
reg [B-1:0] mem1 [0:D] //ascending
reg [B-1:0] mem2 [D:0] //descending
```

When accessing a descending vector via `Signal::getUIntVector<>()`, the return array indicies will be reversed. `mem2_cpp[0] == mem2[D]`. 

> Conventionally, vectors index ranges are defined in ascending order.

### Known Verilator VPI bug

https://github.com/verilator/verilator/issues/5036

The bug reported in the issue above causes erroneous behavior with the current implementation. Aligned bytes of Verilog logic values equal to 0 are overwritten by VPI to 32 (space) when read. And bytes preceded by a leading zero byte are ignored when written. This only effects one VPI format, `vpiStringVal`. Which is the format used by the VerilatorSST. 

As a temporary solution, `Signal` class constructors overwrite bytes with value 32 to 0 before storage. This moves the problem to a less used number, but will need to be resolved in the future. 

```
top.v
...
wire [16:0] moub, arak;
assign moub = 16'h0001;
assign arak = 16'h0100;
```
```
wrapper.cc
...
readPort("moub") //ok (temp fix)
readPort("arak") //ok (temp fix)

writePort("moub",\x0000) //ok
writePort("moub",\x0001) //bad (byte0 = \x00)
writePort("moub",\x1000) //ok
writePort("moub",\x1001) //ok (byte0 = \x10, byte1 = \x01)
```
