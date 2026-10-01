# VERILATOR-SST

**Verilator-SST** is a framework for generating SST subcomponents based on provided (System)Verilog code. It uses **Verilator** to produce a C++ RTL simulator program that is managed by the generated SST Subcomponent. The generated subcomponents can be created with one of two supported interfaces:

### 1. Link Interface

Generates links for each port in the Verilog top module, which can be written or read using the `SST::VerilatorSST::PortEvent` class.

### 2. Direct Interface (C++ API)

Write/read ports using the exposed `writePort`, `writePortAtTick`, and `readPort` functions from a parent component.

> Subcomponents can only be generated with **one** of these interfaces exposed.

- When using the link interface, the `VerilatorComponent` class should be used as the parent component.
- When using the C++ API, specific options must be set to avoid errors (see [Build Options](#build-options)).

### Reading/Writing Ports

There are two modes of reading/writing ports in the Verilated model: **VPI** and **Direct** (not to be confused with the above mentioned Direct C++ API, which is an SST-side interface). Direct reads/writes access the variables directly and may be faster than VPI, with both methods offering consistent behavior.

#### Handling `inout` Ports

`inout` ports are accessible through normal methods. Verilator implements `inout` ports as an `input` port and two `output` ports:

- `<inout_name>__en`: Checks whether the port is being driven by the model.
- `<inout_name>__out`: Reads the value.

The input port is used for writing operations and is assigned the original port name according to the Verilog module. VerilatorSST abstracts these ports and ensures:

- If the signal is **not driven** when a read occurs, or is **driven** when a write occurs, the program will err out.

Tristate ports are not supported in the available versions of Verilator, so `inout` behavior can only be achieved through
the above mentioned three-port abstraction. As a result, `inout` ports **cannot** be accessed properly through VPI.

Therefore, `inout` ports should be read/written by their original names using **Direct** reads/writes.

---

## Dependencies

- [Verilator >v5.022](https://github.com/verilator/verilator/releases/tag/v5.022) (Version 5.026 or greater is required for `inout` port support)
- [SST >16.0.0](https://github.com/sstsimulator/sst-core/releases/tag/v16.0.0_Final)
- Python (>3.6.8)
- CMake (>3.24.2)

---

## Build

To build the project, run the following commands:

```bash
git clone git@github.com:tactcomplabs/verilator-sst.git
cd verilator-sst/
mkdir build && cd build
cmake ../
make
make install
```

To verify that the installation works properly, run:

```bash
make test
```

This will generate two subcomponents for each included example Verilog code (one using the links interface, one using the direct interface) and will use the relevant test component to verify their functionality.

---

## Build Options

### Project Arguments

```bash
-DDISABLE_TESTING                        # Disables testing (enabled by default)
-DENABLE_INOUT_HANDLING=ON               # Allows designs with inout ports (requires Verilator 5.026 or greater)
-DENABLE_CUSTOM_MODULE=ON                # Required to build an external module with CLI model arguments
-DVERILATOR_INCLUDE=<verilator include path>  # Set automatically if not assigned
```

### Model Arguments

```bash
-DVERILOG_SOURCE_DIR=<path to verilog source tree>
-DVERILOG_DEVICE=<name of verilog device to simulate>      # Used in the name of the subcomponent
-DVERILOG_TOP=<name of the top level verilog module>
-DVERILOG_TOP_SOURCES=<list of verilog top source files>
-DVERILATOR_OPTIONS=<additional verilator compilation options>  # Defaults to empty string
-DENABLE_LINK_HANDLING=ON                                  # ON (default): Links build, a link per port. OFF: Direct build (C++ API, no links)
-DCLOCK_PORT_NAME=<name of clock port>                     # Defaults to "clk", used by the Links build
```

### Clocking

- **Direct build:** always clocks itself at its `clockFreq` parameter.
- **Links build:** by default every event on the `clk` link is one clock edge, so the component connected to `clk` is responsible for clocking. Set the runtime parameter `selfClock=1` to have the model toggle its own clock at `clockFreq` instead; the `clk` link must then be left unconnected (and `clockPort` must name the port the model was generated with, `CLOCK_PORT_NAME`). Mismatches are fatal in both directions: `selfClock=1` with `clk` connected, or `selfClock` unset with `clk` unconnected. All other ports remain links either way.
- In both clocking modes one RTL cycle is two Verilator ticks, so `writePortAtTick` offsets mean the same thing. When the model clocks itself, link events may land before or after an edge at the same timestamp, so clients should use the RTL's own handshake or `writePortAtTick`.

---

## Debug

To build with debug options enabled, run:

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ../  # instead of cmake ../
```

---
