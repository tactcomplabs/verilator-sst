#
# Copyright (C) 2017-2026 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#
# accum-driver-direct.py
#
# Your component owns the verilated model directly; no host, no links:
#
#   accumdriverdirect.AccumDriverDirect
#     `- verilatorsstAccumDirect.VerilatorSSTAccumDirect   (RTL inside)
#
# Usage: sst accum-driver-direct.py -- [-t numOps] [-v verbosity] [-a vpi|direct]

import argparse
import sst

parser = argparse.ArgumentParser()
parser.add_argument("-t", "--numops", type=int, default=8)
parser.add_argument("-v", "--verbose", type=int, default=0)
parser.add_argument("-a", "--access", choices=["vpi", "direct"], default="direct")
args = parser.parse_args()

driver = sst.Component("driver0", "accumdriverdirect.AccumDriverDirect")
driver.addParams({
    "verbose": args.verbose,
    "clockFreq": "1GHz",
    "numOps": args.numops,
})

model = driver.setSubComponent("model", "verilatorsstAccumDirect.VerilatorSSTAccumDirect")
model.addParams({
    "useVPI": 1 if args.access == "vpi" else 0,
    "clockPort": "clk",
    # The Direct build clocks itself at this rate (one full RTL cycle per tick).
    "clockFreq": "1GHz",
})
