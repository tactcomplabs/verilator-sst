# VERILATOR-SST

Verilator-SST is a framework for generating SST subcomponents based on provided
(System)Verilog code. It uses Verilator to produce a C++ RTL simulator program
which is then managed by the generated SST Subcomponent. The generated subcomponents
can be created with one of two supported interfaces.

1. Link interface: generate links for each port in the Verilog top
   module which can be written or read respectively using the
`SST::VerilatorSST::PortEvent` class
2. Direct interface (C++ API): write/read ports using the exposed
`writePort`, `writePortAtTick`, and
   `readPort` functions from a parent component

Subcomponents can only be generated with one of these interfaces
exposed. When using the link
interface, the VerilatorComponent class should be used as the parent component.
When using the
C++ API, certain options must be set to avoid errors (see Build Options).

In addition, there are two modes of reading/writing ports in the Verilated
model: VPI and Direct. Direct reads/writes access the variables directly and
may be faster than VPI. Both methods have consistent behavior.

`inout` ports are accessible through the normal methods. Verilator
virtually implements `inout` ports as an `input` port and two `output` ports. The two
output ports are the `<inout_name>__en` port and the `<inout_name>__out`, used
for checking whether the port is being driven by the model and reading the value,
respectively. The input port is used for writing operations and uses the assigned 
portname according to the verilog module. VerilatorSST abstracts these ports and
internally checks if the signal is being driven by the verilated model on read/write.
If it is not driven when a read occurs or is driven when a write occurs, 
the program will err out. As a result, inout ports should be read/written
by their original names.

## Dependencies

- [Verilator >v5.022](https://github.com/verilator/verilator/releases/tag/v5.022) (5.026 is required for inout port support)
- [SST >13.1.0](https://github.com/sstsimulator/sst-core/releases/tag/v13.1.0_Final)
- Python (>3.6.8)
- CMake (>3.24.2)

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
and use the relevant test component to verify their functionality.

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
-DENABLE_INOUT_HANDLING=ON # Allows designs with inout ports (requires Verilator 5.026 or greater)
```

## Debug

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ../ #instead of cmake ../
```
