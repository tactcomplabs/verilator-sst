#
# Copyright (C) 2017-2026 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#
# accum-offload.py
#
# A client component offloads work to the verilated Accum model through a
# small API; the nesting is all subcomponent slots, with no links:
#
#   accumaccel.StubCpu                          slot "accel": AccumAccelAPI
#     `- accumaccel.AccumAdapter                slot "model": VerilatorSSTBase
#          `- verilatorsstAccumDirect.VerilatorSSTAccumDirect   (RTL inside)
#
# Usage: sst accum-offload.py -- [-n numOps] [-b burst] [-p period] [-v verbosity]

import argparse
import sst

parser = argparse.ArgumentParser()
parser.add_argument("-n", "--numops", type=int, default=12)
parser.add_argument("-b", "--burst", type=int, default=4)
parser.add_argument("-p", "--period", type=int, default=40)
parser.add_argument("-v", "--verbose", type=int, default=0)
args = parser.parse_args()

cpu = sst.Component("cpu0", "accumaccel.StubCpu")
cpu.addParams({
    "verbose": args.verbose,
    "clockFreq": "2GHz",
    "numOps": args.numops,
    "burst": args.burst,
    "period": args.period,
})

accel = cpu.setSubComponent("accel", "accumaccel.AccumAdapter")
accel.addParams({
    "verbose": args.verbose,
    "clockFreq": "1GHz",   # rate the adapter steps the RTL handshake
})

model = accel.setSubComponent("model", "verilatorsstAccumDirect.VerilatorSSTAccumDirect")
model.addParams({
    "useVPI": 0,
    "clockPort": "clk",
    "clockFreq": "1GHz",   # the model toggles its own clk at this rate
})
