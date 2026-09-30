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
# Usage: sst accum-offload-links.py -- [-n numOps] [-b burst] [-p period] [-v verbosity]

import argparse
import sst

parser = argparse.ArgumentParser()
parser.add_argument("-n", "--numops", type=int, default=12)
parser.add_argument("-b", "--burst", type=int, default=4)
parser.add_argument("-p", "--period", type=int, default=40)
parser.add_argument("-v", "--verbose", type=int, default=0)
args = parser.parse_args()

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
})

# The host only exists to own the "model" slot.
host = sst.Component("accum0", "verilatorcomponent.VerilatorComponent")
model = host.setSubComponent("model", "verilatorsstAccum.VerilatorSSTAccum")
model.addParams({
    "useVPI": 0,
    "clockPort": "clk",
    "clockFreq": "1GHz",   # irrelevant in the Links build; the adapter drives clk
})

# Connect the *subcomponent* objects (as the test harness does): SST checks
# each endpoint against the ELI port list of the object named here. The
# ports use the verilog port names on both ends.
for p in PORTS:
    sst.Link(f"link_{p}").connect((accel, p, "0ps"), (model, p, "0ps"))
