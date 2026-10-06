#
# Copyright (C) 2017-2026 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#
# accum-driver.py
#
# Shows how to use a verilated model from your own SST component:
#
#   accumdriver.AccumDriver                   <- your component (links only)
#        |  clk reset_l en add accum done
#   verilatorcomponent.VerilatorComponent     <- generic shell, owns the slot
#     `- verilatorsstAccum.VerilatorSSTAccum  <- generated subcomponent (RTL)
#
# Usage: sst accum-driver.py -- [-t numOps] [-v verbosity] [-a vpi|direct]

import argparse
import sst

parser = argparse.ArgumentParser()
parser.add_argument("-t", "--numops", type=int, default=8)
parser.add_argument("-v", "--verbose", type=int, default=0)
parser.add_argument("-a", "--access", choices=["vpi", "direct"], default="direct")
args = parser.parse_args()

PORTS = ["clk", "reset_l", "en", "add", "accum", "done"]

driver = sst.Component("driver0", "accumdriver.AccumDriver")
driver.addParams({
    "verbose": args.verbose,
    "clockFreq": "1GHz",
    "numOps": args.numops,
})

# The host only exists to own the "model" subcomponent slot.
host = sst.Component("accum0", "verilatorcomponent.VerilatorComponent")
model = host.setSubComponent("model", "verilatorsstAccum.VerilatorSSTAccum")
model.addParams({
    "useVPI": 1 if args.access == "vpi" else 0,
    "clockPort": "clk",
    # In the Links build the model's own clock is a no-op; the driver
    # toggles "clk" explicitly, so this frequency is irrelevant.
    "clockFreq": "1GHz",
})

# Ports have the same name on both ends: the verilog port names.
for p in PORTS:
    sst.Link(f"link_{p}").connect((driver, p, "0ps"), (model, p, "0ps"))
