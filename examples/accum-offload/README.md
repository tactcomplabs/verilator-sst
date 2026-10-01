# accum-offload

A client component (`StubCpu`) offloads work to a verilated model (the `Accum` RTL from `test/accum`) through a small API, `AccumAccelAPI`. The same client runs unchanged against **three ways of attaching the RTL**, in four Python configs (the Links one has a self-clocked variant). The point is to show the design choices for putting a verilated model inside a larger system, and what each one costs.

For the underlying Links/Direct model builds and the `useVPI` access choice, see the [top-level README](../../README.md). This directory is the "system design" layer on top of them.

## The client's view

```cpp
// AccumAccelAPI.h
virtual void submit(const Operands &ops, Callback done) = 0;   // never blocks
virtual size_t outstanding() const = 0;
```

The accelerator keeps four running 32-bit totals. `submit` adds four 16-bit operands into them; the callback receives the totals **after that request**. Requests are served in FIFO order. Nothing in the API mentions ports, clocks, reset, or verilator, so the implementation behind it can be swapped without touching the client.

`StubCpu` submits bursts of random operands (`burst` requests every `period` cycles, `numOps` in total), checks every result against its own running totals, and prints a one-line summary with the result count, errors, latency in CPU cycles, and the peak number of outstanding requests.

## The configurations

| Config | What sits in `StubCpu`'s `accel` slot | Where the RTL lives | Run |
|---|---|---|---|
| [`accum-offload.py`](accum-offload.py) | `AccumAdapter` | its `model` slot (Direct build) | `sst accum-offload.py` |
| [`accum-offload-links.py`](accum-offload-links.py) | `AccumLinksAdapter` | a separate `VerilatorComponent` (Links build), over 6 links | `sst accum-offload-links.py` |
| same, self-clocked | `AccumLinksAdapter` with `selfClock` | same, but the model clocks itself and `clk` is unconnected | `sst accum-offload-links.py -- -s` |
| [`accum-offload-component.py`](accum-offload-component.py) | `AccumLinkClient` | a separate, shareable `AccumAccelComponent` (Direct build), over one link per client | `sst accum-offload-component.py` |

### 1. Adapter subcomponent over the Direct build

```
StubCpu ─ slot "accel" ─> AccumAdapter ─ slot "model" ─> VerilatorSSTAccumDirect
```

Everything is one component with nested subcomponents; there are no links. The adapter queues requests and runs the RTL's reset and `en`/`done` handshake from its own clock, in [`AccumEngine`](AccumEngine.h). The model clocks itself. This is the simplest option and what to start from for a CPU or accelerator model that uses RTL as a private functional unit. The one thing it can't do is model the distance between the client and the accelerator.

### 2. Adapter subcomponent over the Links build

```
StubCpu ─ slot "accel" ─> AccumLinksAdapter ──6 links──> VerilatorComponent ─ slot "model" ─> VerilatorSSTAccum
```

Same client, but the model is a Links-build subcomponent in its own `VerilatorComponent` host, and the adapter drives it with `PortEvent`s. It also generates the RTL clock, one cycle per adapter tick. Use this shape when you need the RTL's own ports to be SST links, for example to put something else on them. The config connects the *adapter* object (a subcomponent) to the model's ports, not the CPU.

With `-s` the model clocks itself (`selfClock=1`) and the adapter leaves `clk` unconnected; the other five ports stay links. Setting it on only one side is a fatal error, and `ctest` covers both mismatches.

### 3. The accelerator as its own component

```
StubCpu(s) ─ slot "accel" ─> AccumLinkClient ──link──> AccumAccelComponent ─ slot "model" ─> VerilatorSSTAccumDirect
```

The verilated model is a standalone, shareable component (think: a chiplet). Clients reach it over links with `AccumReq`/`AccumResp` events, through `AccumLinkClient` in their `accel` slot. A callback can't cross a link, so the client tags each request with an id and keeps its callbacks in a map. `AccumAccelComponent` has ports `client0..N-1` and serves all clients in arrival order, using the same `AccumEngine` as option 1.

- **Link latency models the trip to the accelerator.** `-l 10ns` (one way) adds 40 cycles at the stub's 2 GHz to every request, because the round trip is 2 x 10 ns x 2 GHz.
- **One accelerator, several clients:** `-c 3` creates three `StubCpu`s sharing one model.
- **Shared state.** The RTL has one set of running totals, so with several clients each result includes the others' earlier requests. The stubs then check a lower bound (`exact=0`) instead of equality. A real shared accelerator would likely want per-client state or an operation that returns only the caller's contribution; that is a change to the RTL or the API.

## Which one?

| You want | Use |
|---|---|
| A CPU/accelerator model with a private RTL functional unit | 1 (Direct adapter) |
| The RTL's ports available as SST links | 2 (Links adapter) |
| A shared or separately placed accelerator (chiplet, another rank), or modelled latency between client and accelerator | 3 (component) |

Because the API is shared, moving between them is a config change: only the contents of the `accel` slot and the wiring differ.

## Measured behavior

Default arguments (12 requests, bursts of 4 every 40 CPU cycles, 2 GHz CPU, 1 GHz RTL). Latency is in CPU cycles and includes waiting behind earlier requests:

| Config | Latency | Notes |
|---|---|---|
| 1. Direct adapter | 3-27 | |
| 2. Links adapter, and `-s` | 4-28 | one extra cycle for the read round trips; `-s` is identical |
| 3. component, `-l 0ns` | 4-28 | same as the Links adapter |
| 3. component, default `-l 10ns` | 44-62 | +40 for the link |
| 3. component, `-c 3 -l 25ns` | 104-234 | three clients contend; the three summary lines differ |

Requests serialize at about 6 CPU cycles each (three RTL cycles): the Accum RTL handles one operation at a time. "max queue" is the peak number of outstanding requests the client observed; for the component version that includes requests still on the link.

## Options

All three scripts take `-n numOps` (12), `-b burst` (4), `-p period` in CPU cycles (40) and `-v verbosity`.

- `accum-offload-links.py`: `-s` for a self-clocked model.
- `accum-offload-component.py`: `-c clients` (1) and `-l latency` (one-way link latency, default `10ns`).

Pass them after `--`, for example `sst accum-offload-component.py -- -c 3 -l 25ns -v 2`. `-v 2` prints a line per completed request.

## Files

| File | Role |
|---|---|
| `AccumAccelAPI.h` | The client-facing API |
| `StubCpu.{h,cpp}` | The example client; takes any `AccumAccelAPI` in its `accel` slot |
| `AccumEngine.{h,cpp}` | The RTL handshake and request queue (plain C++), shared by options 1 and 3 |
| `AccumAdapter.{h,cpp}` | Option 1: API over the Direct build |
| `AccumLinksAdapter.{h,cpp}` | Option 2: API over the Links build |
| `AccumEvents.h` | `AccumReq` / `AccumResp` link events for option 3 |
| `AccumAccelComponent.{h,cpp}` | Option 3: the shareable accelerator component |
| `AccumLinkClient.{h,cpp}` | Option 3: API over a link to the component |

All of it builds into one SST element library, `accumaccel`.

## Writing your own client or implementation

- **A new client:** load an `AccumAccelAPI` from a slot (`loadUserSubComponent<AccumAccelAPI>("accel")`) and call `submit`. Forward `init(phase)` to it: SST only calls `init` on components, so each level of nesting has to pass it down (client to adapter to model).
- **A new implementation:** derive from `AccumAccelAPI`, register it with `SST_ELI_REGISTER_SUBCOMPONENT(…, SST::VerilatorSST::AccumAccelAPI)`, and put it in the client's slot. A functional (non-RTL) model that implements `submit` is a drop-in replacement.
- **Handshake, not cycle counting:** wait on the RTL's `done` (as `AccumEngine` does) instead of counting cycles. The model's clock and yours are separate handlers, so edge order at a shared timestamp isn't something to depend on.

## Pitfalls

- With a Links model, connect the *subcomponent* objects in `sst.Link(...).connect(...)`. SST checks each endpoint against the port list of the object named there, so naming the parent gives "unknown port".
- `VerilatorComponent` is a primary component that keeps the simulation open for `numCycles` (default 1000 at 1 GHz), so Links runs always report at least 1.001 us regardless of workload. Longer workloads are not cut short.
- The generated model libraries load from `verilator-sst/install/`, so re-run `make install` after regenerating a model.

## Tests

`ctest -R Offload` runs: `AccumOffloadExample`, `AccumOffloadLinksExample`, `AccumOffloadLinksSelfClockExample`, `AccumOffloadComponentExample`, `AccumOffloadComponentSharedExample` (three clients, 25 ns links), and the two `selfClock` mismatch checks.
