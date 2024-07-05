verilator --cc --build --exe --vpi \
--public-flat-rw -Wall --prefix VTop \
-I$(pwd)/vsrc -DCHUNK_SIZE=4 -DNUM_CHUNKS=2 \
-DSCRATCHPAD_BASE=16 vsrc/Scratchpad.sv csrc/testbench.cc
