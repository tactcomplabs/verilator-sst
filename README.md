# VERILATOR-SST

Verilator-SST is a framework for generating SST subcomponents based on provided
(System)Verilog code. It uses Verilator to produce a C++ RTL simulator program
which is then managed by the generated SST Subcomponent. The generated subcomponents
can be created with one of two supported interfaces.

1. Link interface: generate links for each input or output port in the Verilog top
   module which can written or read respectively using the
`SST::VerilatorSST::PortEvent` class
2. Direct interface (C++ API): write/read input/output ports using the exposed
`writePort`, `writePortAtTick`, and
   `readPort` functions from a parent component

Subcomponents can only be generated with one of these interfaces
exposed. When using the link
interface, the VerilatorComponent class should be used as the parent component.
When using the
C++ API, certain options must be set to avoid errors (see Build Options).

## Dependencies

- [Verilator v5.022](https://github.com/verilator/verilator/releases/tag/v5.022)
- [SST 13.1.0](https://github.com/sstsimulator/sst-core/releases/tag/v13.1.0_Final)
- CMake (tested on 3.24.2 and 3.30.2)

## Build

```bash
git clone git@github.com:tactcomplabs/verilator-sst.git
cd verilator-sst/
mkdir build && cd build
cmake ../
make
make install
```

To verify the installation works properly, run

```bash
make test
```
This will generate two subcomponents for each included example Verilog code
(one using links interface, one using direct interface)
and use the included test component to verify their functionality.

### Build Options

```bash
-DVERILOG_SOURCE=<path to verilog source tree>
-DVERILOG_DEVICE=<name of verilog device to simulate>
-DVERILOG_TOP=<name of the top level verilog module>
-DVERILOG_TOP_SOURCES=<list of verilog top source files>
-DVERILATOR_OPTIONS=<additional verilator compilation options>
-DVERILATOR_INCLUDE=<verilator include path>
-DDISABLE_TESTING # Disables testing (which is enabled by default)
-DENABLE_CLK_HANDLING=ON # Generates automatic clock port handling (for C++ API interface)
-DENABLE_LINK_HANDLING=ON # Generates links and link handlers (on by default)
-DCLOCK_PORT_NAME=<name of clock port> # Defaults to "clk", used with ENABLE_LINK_HANDLING
```

## Debug

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ../ #instead of cmake ../
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

