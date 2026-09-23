# Bambu v2023.1 self-contained (XML) testbench support

Status: **working**. This documents the non-DPI alternative pointed to by
`MDPI_TESTBENCH_POC.md`'s "Future direction" section: rather than reusing
bambu's DPI/IPC cosimulation testbench (which has an unresolved
non-determinism issue there), this drives bambu's older, self-contained
testbench generator through `verilator-sst` directly, with no second OS
process and no DPI involved at all.

## Background

Bambu v2023.1 (the last tagged release before the self-contained testbench
generator was removed upstream in favor of the DPI/C-driver flow) supports
`--generate-tb=<file.xml> --simulate --simulator=VERILATOR`. Given an XML file
of concrete input values (see `examples/mm_float/test.xml` in bambu, or
`gen_test_xml.py`/`test.xml` in this example), it generates a single Verilog
file (`testbench_<kernel>_tb.v`) whose top module has **exactly one port**:
`clock`. Everything else — stimulus, memory contents, and the pass/fail
comparison against a native reference execution — is synthesized directly
into that file's own `initial`/`always` blocks. There is no DPI, no second
process, and no IPC: the whole testbench is a single self-contained Verilog
module, driven purely by toggling its one clock port.

That shape — a single-port, clock-only top module — is exactly what
`verilator-sst`'s `ENABLE_CUSTOM_MODULE`/`ENABLE_LINK_HANDLING` custom-module
path (`generate_verilator_component()`) already exists to drive, via a
`VerilatorTestLink` toggling a `clock` `SST::Link` (see `run_forwardKernelTB.py`
for the same pattern used with bambu's DPI-based `bambu_testbench_impl`
module). No DPI library needs linking in at all here (`MDPI_LIBRARY` stays
unset) — this is the first time that custom-module path has been exercised
*without* one.

## How to build and run it

1. Generate a bambu HLS output with the XML testbench flow (from
   `examples/c-to-verilog/3mm-v2/`, with a v2023.1 `bambu` on `PATH` and
   `SDKROOT` exported — see `documentation/install/install_macos.doc` in
   panda-bambu for the full macOS recipe):
   ```bash
   bambu -v3 --print-dot -lm --soft-float --compiler=I386_CLANG13 \
     --device=nangate45 --clock-period=5 --experimental-setup=BAMBU-BALANCED-MP \
     --channels-number=2 --memory-allocation-policy=NO_BRAM --disable-function-proxy \
     --extra-gcc-options=-ffp-contract=off \
     --testbench-extra-gcc-flags="-L${SDKROOT}/usr/lib" \
     --generate-tb=../test.xml --simulate --simulator=VERILATOR --verilator-parallel \
     --top-fname=forward_kernel input.c
   ```
2. Configure/build the `verilator-sst` custom-module component against that
   output (see `output-v2023-seed7/cmake.sh` for the full working invocation):
   ```bash
   cmake -DENABLE_CUSTOM_MODULE=ON \
         -DVERILOG_SOURCE_DIR=./bambu-out \
         -DVERILOG_DEVICE=forwardKernelTB \
         -DVERILOG_TOP=forward_kernel_tb \
         -DVERILOG_TOP_SOURCES=testbench_forward_kernel_tb.v \
         -DVERILATOR_OPTIONS="-Wno-fatal -Wno-lint --timescale-override 1ps/1ps" \
         -DENABLE_LINK_HANDLING=ON \
         -DCLOCK_PORT_NAME=clock \
         ..
   cmake --build .
   ```
3. Run it with `run_forwardKernelTB_v2023.py`, which toggles `clock` the same
   way `run_forwardKernelTB.py` does for the DPI flow:
   ```bash
   sst run_forwardKernelTB_v2023.py -- --cycles 20000
   ```
4. Result: `results.txt` (written by the testbench itself, to the absolute
   path baked in at bambu-generation time) matches a native bambu run
   exactly — `1	17192`, i.e. pass, 17192 cycles.

## The bug this needed: `--timescale-override 1ps/1ps`

Getting to a *completing* simulation (as opposed to just a clean build) needed
one non-obvious fix, worth recording because it isn't specific to this
example — it'll bite any bambu-generated, `$time`-aware Verilog run through
`verilator-sst` without it.

**The symptom**: the build succeeded and the SST run completed without
error, but `results.txt` stayed empty — the testbench's internal state
machine never progressed past its own startup gate
(`if (currTime > `INIT_TIME`) ...`).

**The cause**: two independent, unrelated notions of "time" are in play.
`verilator-sst`'s `VerilatorTestLink` paces its own SST-level clock (real SST
scheduled time, `clockFreq` param), but the Verilated model's own `$time`
system function only ever advances when the generated clock-port event
handler calls `ContextP->timeInc(1)` — one raw *precision*-unit tick per
driven clock edge, hardcoded, with no relationship to SST's own clock rate.

Separately, bambu's testbench generator (`testbench_generation_base_step.cpp`)
always emits `` `timescale 1ns / 1ps `` into the generated Verilog — but
deliberately hardcodes `` `define HALF_CLOCK_PERIOD 1`` (rather than the real
half-period computed from `--clock-period`) *specifically* when
`--simulator=VERILATOR`. That `1` is meant as "1 raw tick," not "1
nanosecond" — and it's only correct under a *second*, paired piece of bambu
behavior: at configure time, bambu probes whether the local `verilator`
supports `--timescale-override`, and if so unconditionally sets
`--timescale-override 1ps/1ps` on every Verilator invocation it makes
(`BambuParameter.cpp`/`EucalyptusParameter.cpp`). That override collapses the
design's timeunit down to match its timeprecision (both 1ps), which is what
makes the generated Verilog's own `1`s mean "1 raw tick" consistently.

`verilator-sst`'s build scripts never passed that flag, so Verilator fell
back to the file's literal `1ns / 1ps` timescale declaration — timeunit
1000x coarser than timeprecision. `HALF_CLOCK_PERIOD=1` (meant as 1 raw tick)
was reinterpreted as 1 real nanosecond = 1000 raw ticks, and every one of our
driven clock toggles (which each contribute exactly 1 raw tick via
`timeInc(1)`) only advanced the design's notion of time by 0.001 of what it
expected. `INIT_TIME=100` (meant as 100 raw ticks, i.e. 50 of our clock
edges) effectively became "100 real nanoseconds" (50,000 of our edges), and
full completion (bambu's native 17,192 cycles) would have needed roughly
17 million driven cycles instead.

**The fix**: add the same `--timescale-override 1ps/1ps` flag bambu's own
native Verilator invocation always uses, to `VERILATOR_OPTIONS` in `cmake.sh`.
No changes to `verilator-sst`'s C++ or shell codegen were needed — the
existing `timeInc(1)` was already correct; it just needed the timescale
Verilator compiled the design under to actually match what bambu assumed.

## A minor caveat: `$finish` doesn't stop the driver

The generated testbench calls Verilog's `$finish` once it completes. Under
Verilator, `$finish` just sets an internal flag rather than killing the host
process, so `run_forwardKernelTB_v2023.py`'s driver keeps calling `eval()` on
every subsequent queued clock toggle for the rest of its `--cycles` budget
(each one re-printing the `$finish` notice, harmlessly). The correct result
is already written to `results.txt` well before that; it's cosmetic, not a
correctness issue. Passing a `--cycles` value not too much larger than
bambu's own reported native cycle count (visible in the `bambu-xml-sim.log`
for a given testbench, e.g. 17192 here) keeps the noise down and the run
fast.
