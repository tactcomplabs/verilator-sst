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
