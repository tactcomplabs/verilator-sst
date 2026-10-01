# Examples: using verilated models from your own components

These examples show how to write SST components that *use* a verilated
model (here, the `Accum` test RTL) rather than test it. They are built when
testing is enabled, because they rely on the `verilatorsstAccum` and
`verilatorsstAccumDirect` subcomponent libraries generated under `test/`.
Each one has a `ctest` entry (`ctest -R Accum`).

None of the examples include or link anything generated from the verilog.
They only use `verilatorSSTAPI.h` (`PortEvent`, `VerilatorSSTBase`), so the
same code works against any model exposing the same ports.

## Two builds of a model, two ways to attach it

| Build | Subcomponent | Who toggles `clk` | How you talk to it |
|---|---|---|---|
| **Links** | `verilatorsstAccum.VerilatorSSTAccum` | you (write `clk` events) | `PortEvent`s over one SST link per RTL port |
| **Direct** | `verilatorsstAccumDirect.VerilatorSSTAccumDirect` | the model itself | `writePort()` / `readPort()` method calls |

- **Links:** the subcomponent calls `configureLink` on its parent's ports, so
  it must sit in a `verilatorcomponent.VerilatorComponent` host (which only
  owns the `model` slot), and every port must be connected in the config. Link
  endpoints in Python are the *subcomponent* objects, since SST checks each
  endpoint against that object's ELI port list.
- **Direct:** no links. The component that owns the `model` slot calls the
  model directly. It must forward `init(phase)` to it, because SST only calls
  `init` on components. Its clock is a separate clock handler from yours, so
  don't depend on which edge fires first at a timestamp; use the RTL's own
  handshake (here, `done`) or `writePortAtTick`.

Payloads are little-endian bytes sized by port width and depth. For `Accum`,
`add` is 4 x 16 bits (8 bytes) and `accum` is 4 x 32 bits (16 bytes).

## The examples

| Directory | Pattern |
|---|---|
| [`accum-driver/`](accum-driver) | **Links, testbench style.** A component drives the six ports directly: clock, reset, operands, reads. The closest to what the test harness does. |
| [`accum-driver-direct/`](accum-driver-direct) | **Direct, component owns the model.** Same job, using method calls and the `done` handshake. |
| [`accum-offload/`](accum-offload) | **Client + adapter** (see its [README](accum-offload/README.md)). A CPU-like component offloads work through a small API; an adapter hides the RTL behind it. Three ways to attach the RTL, same client, in one library. |

### accum-offload

This is the pattern to copy for a CPU or accelerator model that uses RTL
as a functional unit. The client sees only `AccumAccelAPI`
(`submit(ops, callback)`: FIFO, non-blocking, results by callback) and knows
nothing about ports, clocks or reset. Nested subcomponent slots carry the
wiring:

```
Direct:  StubCpu -- slot "accel" --> AccumAdapter -- slot "model" --> VerilatorSSTAccumDirect

Links:   StubCpu -- slot "accel" --> AccumLinksAdapter --6 links--> VerilatorComponent -- slot "model" --> VerilatorSSTAccum
```

```
Component:  StubCpu -- slot "accel" --> AccumLinkClient --link--> AccumAccelComponent -- slot "model" --> VerilatorSSTAccumDirect
```

[`accum-offload.py`](accum-offload/accum-offload.py),
[`accum-offload-links.py`](accum-offload/accum-offload-links.py) and
[`accum-offload-component.py`](accum-offload/accum-offload-component.py) differ
only in what sits in the CPU's `accel` slot and the wiring; `StubCpu` is
identical.

#### The accelerator as its own component

`AccumAccelComponent` is the verilated model as a standalone, shareable
component (think: a chiplet). Clients reach it over links with `AccumReq` /
`AccumResp` events, through `AccumLinkClient` in their `accel` slot. A
callback can't cross a link, so the client tags each request with an id and
keeps its callbacks in a map. The component has `client0..N-1` ports and serves
all clients in arrival order.

- **Modelled latency:** the link latency is the trip to the accelerator and back.
  `-l 10ns` adds 40 cycles at the stub's 2 GHz (2 x 10 ns x 2 GHz) to every
  request; `-l 0ns` matches the Links adapter's numbers (the Direct adapter is one cycle faster).
- **Shared state:** the RTL has one set of running totals, so with several
  clients each result includes the others' earlier requests (`-c 3`). The stubs
  then check a lower bound (`exact=0`) instead of equality.
- **Shared code:** all three implementations run requests through the same
  `AccumEngine` (queue plus reset and `en`/`done` handshake), except the
  Links adapter, which drives the RTL from PortEvents.

## Simulated time in the Links examples

The Links examples report about 1.001 us of simulated time however little work they do. That comes from the `VerilatorComponent` host, a primary component that keeps the simulation open for `numCycles` of its clock (default 1000 at 1 GHz), not from the work. The simulation still waits for your own components to finish, so longer workloads are not cut short; set `numCycles` on the host if you want shorter runs.

## Choosing

- Want a self-contained check of a model from your own code? `accum-driver*`.
- Building something larger that *uses* the model? `accum-offload`, with the
  Direct adapter unless you need the model in a separate component.
- Accelerator that several clients share, that needs link latency, or that is
  its own placed unit (a chiplet, another rank)? `AccumAccelComponent` over
  links, using the Direct model behind it.
- Need to see the RTL's own ports as SST links? Use the Links build, as in
  `accum-offload-links.py`.

## Running

```bash
sst examples/accum-driver/accum-driver.py -- -t 20
sst examples/accum-driver-direct/accum-driver-direct.py -- -t 20
sst examples/accum-offload/accum-offload.py -- -n 12 -b 4 -p 40
sst examples/accum-offload/accum-offload-links.py -- -n 12 -b 4 -p 40
sst examples/accum-offload/accum-offload-component.py -- -c 3 -l 25ns
```

The element libraries (`accumdriver`, `accumdriverdirect`, `accumaccel`) are
registered by `make install` like the rest of the project. Add `-v 2` for a
per-result trace.
