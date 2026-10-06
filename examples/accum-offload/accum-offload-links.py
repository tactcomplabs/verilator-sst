#
# Copyright (C) 2017-2026 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#
# accum-offload-links.py
#
# Same StubCpu and AccumAccelAPI as accum-offload.py, but the RTL is the
# Links build in its own component, reached over six SST links:
#
#   accumaccel.StubCpu                      slot "accel": AccumAccelAPI
#     `- accumaccel.AccumLinksAdapter        (drives the RTL over links)
#          |  clk reset_l en add accum done
#   verilatorcomponent.VerilatorComponent
#     `- verilatorsstAccum.VerilatorSSTAccum   (RTL inside)
#
# Only the adapter and the wiring differ from the Direct version; the CPU
# is unchanged.
#
# Usage: sst accum-offload-links.py -- [-n numOps] [-b burst] [-p period] [-v verbosity] [-s]
#
# With -s the model clocks itself (selfClock=1) and the adapter does not drive
# or connect clk; all the other ports are still links.

import argparse
import sst

parser = argparse.ArgumentParser()
parser.add_argument("-n", "--numops", type=int, default=12)
parser.add_argument("-b", "--burst", type=int, default=4)
parser.add_argument("-p", "--period", type=int, default=40)
parser.add_argument("-v", "--verbose", type=int, default=0)
parser.add_argument("-s", "--self-clock", action="store_true",
                    help="the model clocks itself instead of taking clk link events")
# Only for exercising the error checks: set the two sides independently.
parser.add_argument("--model-self-clock", type=int, choices=[0, 1], default=None)
parser.add_argument("--adapter-self-clock", type=int, choices=[0, 1], default=None)
args = parser.parse_args()

model_self = int(args.self_clock) if args.model_self_clock is None else args.model_self_clock
adapter_self = int(args.self_clock) if args.adapter_self_clock is None else args.adapter_self_clock

PORTS = ["clk", "reset_l", "en", "add", "accum", "done"]

cpu = sst.Component("cpu0", "accumaccel.StubCpu")
cpu.addParams({
    "verbose": args.verbose,
    "clockFreq": "2GHz",
    "numOps": args.numops,
    "burst": args.burst,
    "period": args.period,
})

accel = cpu.setSubComponent("accel", "accumaccel.AccumLinksAdapter")
accel.addParams({
    "verbose": args.verbose,
    "clockFreq": "1GHz",   # one RTL clock cycle per tick
    "selfClock": adapter_self,  # 1: do not drive or connect clk
})

# The host only exists to own the "model" slot.
host = sst.Component("accum0", "verilatorcomponent.VerilatorComponent")
model = host.setSubComponent("model", "verilatorsstAccum.VerilatorSSTAccum")
model.addParams({
    "useVPI": 0,
    "clockPort": "clk",
    # With selfClock=0 the model is clocked by clk link events and clockFreq
    # is unused. With selfClock=1 the model toggles clk itself at clockFreq.
    "selfClock": model_self,
    "clockFreq": "1GHz",
})

# Connect the *subcomponent* objects (as the test harness does): SST checks
# each endpoint against the ELI port list of the object named here. The
# ports use the verilog port names on both ends.
for p in PORTS:
    if p == "clk" and adapter_self:
        continue   # the model clocks itself; clk stays unconnected
    sst.Link(f"link_{p}").connect((accel, p, "0ps"), (model, p, "0ps"))
