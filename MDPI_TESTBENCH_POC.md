# Bambu DPI testbench support (proof of concept)

Status: **experimental / proof of concept**. This was built to answer a
specific question (can a bambu-generated `*_testbench.v`, not just the bare
DUT, be exercised through `verilator-sst`?) and is not recommended as a
long-term integration path. See "Known issue" and "Future direction" below.

## Background

A bambu-generated `bambu_testbench.v` (produced with `--generate-tb=...
--simulate --simulator=VERILATOR`) does not contain its own stimulus, memory
contents, or correctness checking. All of that lives in a separate C/C++
**driver process** that bambu also builds, and the RTL side only talks to it
through four `import "DPI-C"` functions: `m_next`, `m_read`, `m_write`, and
`m_fini` (see bambu's `libmdpi` at
`panda/install/share/panda/libmdpi/mdpi.c`). Those functions are not stubs —
every one of them blocks on a real inter-process round trip (a memory-mapped
file plus a wait/signal handshake) to the driver binary, which holds the
actual input arrays and runs the original (non-HLS'd) C reference function to
produce the "gold" values it compares against.

Without linking a real implementation of those four functions into the
generated SST component, calling any of them jumps into an unresolved symbol
and segfaults immediately (this is what happens if you build a
`*_testbench.v` top module without `MDPI_LIBRARY` set).

## What this adds

- A new `MDPI_LIBRARY` CMake variable (top-level `CMakeLists.txt`) and an
  optional 8th positional argument to `generate_verilator_component()`
  (`verilator-sst-element/CMakeLists.txt`), so a specific
  `-DENABLE_CUSTOM_MODULE=ON` build can link a prebuilt DPI implementation
  (e.g. bambu's own `HLS_output/beh_sim/libmdpi.so`) into just that one
  generated component.
- Deliberately **not** wired into the generic `generate_verilator_component`
  call sites in `test/CMakeLists.txt`: `mdpi.c`'s `m_init`/`m_fini` are
  `__attribute__((constructor/destructor))` functions that kick off IPC setup
  the moment the shared library is loaded, so linking it into every built-in
  test component (Counter, Accum, etc. — none of which use DPI) would trigger
  pointless IPC initialization for unrelated targets.

## How it was exercised (3mm-v2 forward_kernel testbench)

1. Configure/build with the DPI implementation linked in:
   ```bash
   cmake -DMDPI_LIBRARY=<path>/output/bambu/baseline/HLS_output/beh_sim/libmdpi.so .
   cmake --build .
   ```
2. Bambu's driver binary (already built at
   `output/bambu/baseline/HLS_output/simulation/testbench`) and the SIM-side
   `libmdpi.so` linked above were both compiled with the **same
   compile-time-constant, relative** `__M_IPC_FILENAME`
   (`HLS_output/simulation/panda_sock`). Both processes must therefore be run
   from the same working directory — `output/bambu/baseline/` — for that
   relative path (and `libmdpi.so`'s own relative install name) to resolve
   to the same file.
3. Set `M_IPC_SIM_CMD` to the command that runs the SST simulation, then run
   bambu's driver binary as the top-level process — it `fork()`/`exec()`s
   whatever is in `M_IPC_SIM_CMD` as its child instead of its own
   `Vbambu_testbench`:
   ```bash
   cd output/bambu/baseline
   export M_IPC_SIM_CMD="sst <path>/run_forwardKernelTB.py -- --cycles 30000 2>&1 | tee sst_run.log; exit \${PIPESTATUS[0]};"
   ./HLS_output/simulation/testbench
   ```
4. Result: the driver and the SST-embedded RTL **do** synchronize over
   bambu's real IPC channel — real memory reads/writes cross the boundary,
   and `m_fini()` returns a real pass/fail verdict from bambu's own
   golden-model comparison (written to `results.txt`, same format as a
   native bambu run: `<start>|<end>,\n<retcode>`).

## Known issue: non-deterministic mismatch

Running the exact same driver/SST pairing twice produced two different
results:

- Run 1: `results.txt` = `7|24251,\n1` (roughly), memory parameter 6 wrong.
- Run 2: `results.txt` = `8|20952,\n1`, memory parameter 0 wrong instead.

A native bambu run of this same RTL and testbench already passes
(`output/bambu/baseline/07_results.txt` = `7|24251,\n0`). Since the *which
parameter fails* changes between otherwise-identical runs, this points to a
timing/synchronization issue in how SST paces clock edges relative to the
real, blocking cross-process IPC calls `m_read`/`m_write`/`m_next` make on
every clock edge — not a bug in the RTL itself. `verilator-sst`'s existing
tests never exercise this path because none of them use DPI; driving a plain
clock signal through `VerilatorTestLink` (no blocking cross-process calls in
the middle of `eval()`) is what all of them do today. There is also a
segfault-on-abort during the `STATE_ABORT` cleanup path once a mismatch is
detected, secondary to the same issue.

This has not been root-caused. Given the fix is architecture-specific to
bambu's `mdpi_ipc` wait/signal protocol and how it composes with SST's event
loop, and given the intended use case here (single example project in
`soda-benchmarks`) doesn't justify the investigation, this is left as a known
limitation rather than fixed.

## Future direction

Rather than reusing bambu's DPI-based cosimulation testbench, the more
promising longer-term path is probably to look at bambu's *testbench
generation* step itself (`--generate-tb=...`) — e.g. generating a
verilator-sst-native testbench directly, without the DPI/IPC layer, so
stimulus and checking happen in-process instead of through a second OS
process. That avoids this class of timing issue entirely rather than working
around it.
