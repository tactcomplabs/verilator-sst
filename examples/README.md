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
| [`accum-offload/`](accum-offload) | **Client + adapter.** A CPU-like component offloads work through a small API; an adapter hides the RTL behind it. Two adapters, same client, in one library. |

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

[`accum-offload.py`](accum-offload/accum-offload.py) and
[`accum-offload-links.py`](accum-offload/accum-offload-links.py) differ only in
the adapter and the wiring; `StubCpu` is identical. To model bus or NoC latency
between client and accelerator, add a delay in the adapter before the callback
fires.

## Choosing

- Want a self-contained check of a model from your own code? `accum-driver*`.
- Building something larger that *uses* the model? `accum-offload`, with the
  Direct adapter unless you need the model in a separate component.
- Model must be a separately placed component (for example across
  ranks), or you need to see its ports as SST links? Use Links.

## Running

```bash
sst examples/accum-driver/accum-driver.py -- -t 20
sst examples/accum-driver-direct/accum-driver-direct.py -- -t 20
sst examples/accum-offload/accum-offload.py -- -n 12 -b 4 -p 40
sst examples/accum-offload/accum-offload-links.py -- -n 12 -b 4 -p 40
```

The element libraries (`accumdriver`, `accumdriverdirect`, `accumaccel`) are
registered by `make install` like the rest of the project. Add `-v 2` for a
per-result trace.
