# VERILATOR-SST

**Verilator-SST** generates SST subcomponents from (System)Verilog. It runs **Verilator** on your design to produce a C++ RTL simulator, and wraps that simulator in an SST subcomponent that exposes the design's top-level ports to the rest of an SST simulation.

## Two independent choices

There are two separate decisions to make, and they are easy to confuse because both can be called "direct":

| | Choice | Options | Decided | Matters to you? |
|---|---|---|---|---|
| **1** | How the SST side talks to the model (the *SST interface*) | **Links** or **Direct** | at build time, once per generated model | **Yes.** It changes the generated subcomponent and how you wire it. |
| **2** | How the subcomponent reads/writes the Verilated model's ports (the *port access* method) | **Direct** variable access or **VPI** | at run time, the `useVPI` parameter | **Rarely.** The default is fine. |

Any SST interface works with either port access method; all four combinations are exercised by the tests (`inout` ports are the one exception, see [Port access](#port-access-direct-vs-vpi)).

### 1. The SST interface (build time)

- **Links** (default): the subcomponent has one SST link per top-level port. You read and write a port by sending an `SST::VerilatorSST::PortEvent` on its link. The subcomponent must sit in a `verilatorcomponent.VerilatorComponent`, which only exists to own the subcomponent slot (see [Using a Links model](#using-a-links-model)).
- **Direct** (C++ API): no links. The component that owns the subcomponent slot calls `writePort`, `writePortAtTick` and `readPort` on it directly. Use this when a single component of your own uses the RTL as a functional unit (see [Using a Direct model](#using-a-direct-model)).

A generated subcomponent has **one** of these interfaces, not both. The Direct model's SST name carries a `Direct` suffix (for example `verilatorsstAccumDirect.VerilatorSSTAccumDirect`), so you can build both flavors of the same design side by side.

### 2. The port access method (run time)

Every generated model is built with both access methods compiled in. `useVPI=0` (default) reads and writes the Verilated model's variables directly; `useVPI=1` goes through Verilator's VPI. Behavior is consistent between them, and direct access may be faster. Unless you have a reason to use VPI, leave it at the default. Details and the `inout` caveat are [below](#port-access-direct-vs-vpi).

---

## Dependencies

- [Verilator >v5.022](https://github.com/verilator/verilator/releases/tag/v5.022) (Version 5.026 or greater is required for `inout` port support)
- [SST >16.0.0](https://github.com/sstsimulator/sst-core/releases/tag/v16.0.0_Final)
- Python (>3.6.8)
- CMake (>3.24.2)

`sst`, `sst-config` and `verilator` must be on your `PATH`. If CMake can't find Verilator, set `VERILATOR_ROOT`.

---

## Build

```bash
git clone git@github.com:tactcomplabs/verilator-sst.git
cd verilator-sst/
mkdir build && cd build
cmake ../
make
make install
```

Builds must be out-of-tree. `make install` copies the generated libraries into `verilator-sst/install/` and registers them with SST (`sst-register`).

To verify the installation:

```bash
make test
```

This generates a Links and a Direct subcomponent for each example Verilog design under `test/`, and runs each with both port access methods against a test component. It also builds and runs the [examples](examples/README.md).

---

## Using your own Verilog

Configure with `ENABLE_CUSTOM_MODULE` and describe your design:

```bash
cmake -DENABLE_CUSTOM_MODULE=ON \
      -DVERILOG_SOURCE_DIR=/path/to/rtl \
      -DVERILOG_TOP=MyTop \
      -DVERILOG_TOP_SOURCES=MyTop.sv \
      -DVERILOG_DEVICE=MyDevice \
      -DCLOCK_PORT_NAME=clk \
      ../
make
make install
```

Add `-DENABLE_LINK_HANDLING=OFF` for a Direct model instead of a Links model. A single configure generates one of the two; to get both flavors of one design, call `generate_verilator_component` (in `verilator-sst-element/CMakeLists.txt`) once per interface from your own CMake. Add `-DDISABLE_TESTING=ON` to skip the bundled tests and examples.

The result is an SST element library named after `VERILOG_DEVICE`:

| Build | Library | Subcomponent type in your Python config |
|---|---|---|
| Links | `verilatorsst<Device>` | `verilatorsst<Device>.VerilatorSST<Device>` |
| Direct | `verilatorsst<Device>Direct` | `verilatorsst<Device>Direct.VerilatorSST<Device>Direct` |

### Generated subcomponent parameters

| Parameter | Default | Meaning |
|---|---|---|
| `clockPort` | *(required)* | Name of the Verilog clock port (must be a port of the design) |
| `clockFreq` | `1GHz` | Rate at which the model clocks itself. Used by the Direct build always, and by the Links build only with `selfClock=1`. |
| `useVPI` | `0` | Port access method: `0` direct variable access, `1` VPI. See [below](#port-access-direct-vs-vpi). |
| `resetVals` | none | List of `"port:value"` strings (decimal) written to ports during SST `init` |
| `selfClock` | `0` | **Links build only.** `1` makes the model clock itself; the `clk` link must then be unconnected. See [Clocking](#clocking). |
| `verbose` | `0` | Output verbosity |

The Verilog port names are the SST port names (Links) and the port names you pass to `writePort`/`readPort` (Direct). Payloads are byte vectors, little-endian, sized by the port's width and depth: for example an `[15:0] add[4]` input is 8 bytes and a `[31:0] accum[4]` output is 16 bytes.

---

## Using a Links model

Wire your component's ports to the model's ports with one `sst.Link` per port. The model lives in a `VerilatorComponent`, a small shell that owns the `model` slot:

```python
import sst

PORTS = ["clk", "reset_l", "en", "add", "accum", "done"]

host = sst.Component("accum0", "verilatorcomponent.VerilatorComponent")
host.addParams({"numCycles": 1000})   # see note below
model = host.setSubComponent("model", "verilatorsstAccum.VerilatorSSTAccum")
model.addParams({"clockPort": "clk"})

mine = sst.Component("driver0", "mylib.MyDriver")   # your component

for p in PORTS:
    # connect the *subcomponent* object: SST checks each endpoint against the
    # port list of the object named here
    sst.Link(f"link_{p}").connect((mine, p, "0ps"), (model, p, "0ps"))
```

From your component:

- **Write** a port: `link->send(new PortEvent(bytes))`. `PortEvent(bytes, tick)` delays the write by `tick` Verilator ticks.
- **Read** a port: `link->send(new PortEvent())`; the model answers on the same link with a `PortEvent` holding the bytes, so configure the link with a handler.
- **Every** top-level port must be connected, including `clk`, unless you use `selfClock=1`.
- **Clock:** by default *you* generate the clock, by writing `clk` high then low once per cycle; see [Clocking](#clocking).

`VerilatorComponent` registers as a primary component and keeps the simulation from ending for `numCycles` cycles of its own clock (default 1000 at 1 GHz). A short test will therefore always report at least `numCycles` cycles of simulated time. Set `numCycles` to cover your workload, or smaller to let the simulation end sooner.

A complete working version is in [examples/accum-driver](examples/accum-driver) (drive the ports yourself) and [examples/accum-offload](examples/accum-offload) (a client behind a small API).

---

## Using a Direct model

Your component owns the model in a subcomponent slot and calls it:

```python
import sst

cpu = sst.Component("cpu0", "mylib.MyComponent")   # your component
model = cpu.setSubComponent("model", "verilatorsstAccumDirect.VerilatorSSTAccumDirect")
model.addParams({"clockPort": "clk", "clockFreq": "1GHz"})
```

```cpp
#include "verilatorSSTAPI.h"
using namespace SST::VerilatorSST;

// in your component's ELI:
SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS(
  {"model", "Verilator model", "SST::VerilatorSST::VerilatorSSTBase"})

// constructor
model = loadUserSubComponent<VerilatorSSTBase>("model");

// SST only calls init() on your component, so forward it
void init(unsigned phase) { model->init(phase); }

// use it
model->writePort("en", {1});                   // takes effect immediately
model->writePortAtTick("add", bytes, 2);       // 2 Verilator ticks from now
std::vector<uint8_t> sum = model->readPort("accum");
```

Also available: `isNamedPort`, `getPortsNames`, `getPortWidth`, `getPortDepth`, `getPortType`, `getResetVal`, `getCurrentTick`.

The model clocks itself, so you do not write the clock port. Its clock is a separate clock handler from yours, so do not depend on which edge fires first at a given timestamp: wait on the RTL's own handshake signal, or use `writePortAtTick`.

A complete working version is in [examples/accum-driver-direct](examples/accum-driver-direct) and [examples/accum-offload](examples/accum-offload).

---

## Clocking

- **Direct build:** always clocks itself at its `clockFreq` parameter.
- **Links build:** by default every event on the `clk` link is one clock edge, so the component connected to `clk` is responsible for clocking. Set the runtime parameter `selfClock=1` to have the model toggle its own clock at `clockFreq` instead; the `clk` link must then be left unconnected (and `clockPort` must name the port the model was generated with, `CLOCK_PORT_NAME`). Mismatches are fatal in both directions: `selfClock=1` with `clk` connected, or `selfClock` unset with `clk` unconnected. All other ports remain links either way.
- In both clocking modes one RTL cycle is two Verilator ticks, so `writePortAtTick` offsets mean the same thing. When the model clocks itself, link events may land before or after an edge at the same timestamp, so clients should use the RTL's own handshake or `writePortAtTick`.

#### Differences from earlier Links builds

With `selfClock` unset (the default), a Links model behaves as it did before `selfClock` existed, with one change: it no longer registers an SST clock. Previously every Links model registered a clock at `clockFreq` whose handler did nothing.

- Simulation results are unaffected, since that clock did no work and did not affect when the simulation ends. Event counts and runtime drop slightly because the empty events are gone, so profiling or event-count numbers will look different from older runs.
- `clockFreq` is still accepted on a Links model but is ignored unless `selfClock=1`.
- `ENABLE_CLK_HANDLING` was removed; it no longer controlled anything. Use `ENABLE_LINK_HANDLING` to choose between the Links build (ON, the default) and the Direct build (OFF).
- If you regenerate a model, rebuild and re-install its library: the clock registration is generated code.

---

## Port access: direct vs VPI

This is the model-side choice, independent of Links vs Direct above. Both methods are compiled into every generated model (Verilator is always run with `--vpi --public-flat-rw`) and you pick one per model instance with `useVPI`:

- **`useVPI=0` (default):** reads and writes the Verilated model's variables directly. Possibly faster.
- **`useVPI=1`:** goes through Verilator's VPI, looking ports up by name.

Both give the same results; the test suite runs every example both ways. For most users it makes no difference, and the default is the right choice. The one real limitation is `inout` ports.

### Handling `inout` ports

`inout` ports are accessible through normal methods. Verilator implements `inout` ports as an `input` port and two `output` ports:

- `<inout_name>__en`: Checks whether the port is being driven by the model.
- `<inout_name>__out`: Reads the value.

The input port is used for writing operations and is assigned the original port name according to the Verilog module. VerilatorSST abstracts these ports and ensures:

- If the signal is **not driven** when a read occurs, or is **driven** when a write occurs, the program will err out.

Tristate ports are not supported in the available versions of Verilator, so `inout` behavior can only be achieved through the above mentioned three-port abstraction. As a result, `inout` ports **cannot** be accessed properly through VPI, so a design with `inout` ports must use direct access (`useVPI=0`). The tests skip the VPI cases for the `Pin` example for this reason. `inout` support also requires `-DENABLE_INOUT_HANDLING=ON`.

---

## Build options

### Project options

| Option | Default | Meaning |
|---|---|---|
| `DISABLE_TESTING=ON` | off | Skip the bundled tests and examples |
| `ENABLE_INOUT_HANDLING=ON` | OFF | Support designs with `inout` ports (Verilator 5.026+) |
| `ENABLE_CUSTOM_MODULE=ON` | OFF | Build the module described by the model options below |
| `VERILATOR_INCLUDE=<path>` | derived from Verilator | Verilator include path |

### Model options (with `ENABLE_CUSTOM_MODULE=ON`)

| Option | Default | Meaning |
|---|---|---|
| `VERILOG_SOURCE_DIR` | `../verilog` | Path to the Verilog source tree |
| `VERILOG_TOP` | `Top` | Name of the top-level module |
| `VERILOG_TOP_SOURCES` | `top.v` | Top-level source file(s) |
| `VERILOG_DEVICE` | `Device` | Name used in the generated library and subcomponent |
| `VERILATOR_OPTIONS` | empty | Extra Verilator options (for example `-GPARAM=value`) |
| `MDPI_LIBRARY` | empty | Path to a prebuilt DPI implementation (e.g. bambu's `libmdpi.so`) for top modules that import its DPI-C functions (`m_next`, `m_read`, `m_write`, `m_fini`, `m_state`). Experimental; see [MDPI_TESTBENCH_POC.md](MDPI_TESTBENCH_POC.md). |
| `ENABLE_LINK_HANDLING` | ON | ON: Links build. OFF: Direct build (C++ API, no links). This is the SST interface choice. |
| `CLOCK_PORT_NAME` | `clk` | Name of the clock port; used by the Links build |

The port access method (`useVPI`) is **not** a build option; it is a run-time subcomponent parameter.

---

## Debug

To build with debug options enabled, run:

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ../  # instead of cmake ../
```

---

## Examples

[`examples/`](examples/README.md) has working components that use the generated models: a driver for each SST interface, and a CPU-style client that offloads to the model through a small API (with the model attached via Direct, Links, or as a separate shared component). Each runs under `make test`.
