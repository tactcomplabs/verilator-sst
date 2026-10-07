#
# Copyright (C) 2017-2026 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#
# accum-offload-component.py
#
# The verilated model as its own component (a "chiplet") that one or more
# clients reach over SST links. The client code (StubCpu) is the same as in the
# other accum-offload examples; only the thing in its "accel" slot changes.
#
#   accumaccel.StubCpu  (x N)      slot "accel": AccumAccelAPI
#     `- accumaccel.AccumLinkClient
#          | link  <--- latency models the trip to the accelerator --->
#   accumaccel.AccumAccelComponent   ports client0..N-1
#     `- verilatorsstAccumDirect.VerilatorSSTAccumDirect   (RTL inside)
#
# Usage: sst accum-offload-component.py -- [-c clients] [-l latency] [-n numOps]
#            [-b burst] [-p period] [-v verbosity]
#
# Note the RTL keeps ONE set of running totals, so with more than one client
# each result includes the other clients' earlier requests. The StubCpus then
# check a lower bound instead of an exact match.

import argparse
import sst

parser = argparse.ArgumentParser()
parser.add_argument("-c", "--clients", type=int, default=1)
parser.add_argument("-l", "--latency", default="10ns", help="one-way link latency, e.g. 10ns")
parser.add_argument("-n", "--numops", type=int, default=12)
parser.add_argument("-b", "--burst", type=int, default=4)
parser.add_argument("-p", "--period", type=int, default=40)
parser.add_argument("-v", "--verbose", type=int, default=0)
args = parser.parse_args()

accel = sst.Component("accel0", "accumaccel.AccumAccelComponent")
accel.addParams({
    "verbose": args.verbose,
    "clockFreq": "1GHz",          # rate the RTL handshake is stepped
    "num_clients": args.clients,
})
model = accel.setSubComponent("model", "verilatorsstAccumDirect.VerilatorSSTAccumDirect")
model.addParams({
    "useVPI": 0,
    "clockPort": "clk",
    "clockFreq": "1GHz",          # the model toggles its own clk at this rate
})

for i in range(args.clients):
    cpu = sst.Component(f"cpu{i}", "accumaccel.StubCpu")
    cpu.addParams({
        "verbose": args.verbose,
        "clockFreq": "2GHz",
        "numOps": args.numops,
        "burst": args.burst,
        "period": args.period,
        "seed": i + 1,
        "exact": 1 if args.clients == 1 else 0,
    })
    client = cpu.setSubComponent("accel", "accumaccel.AccumLinkClient")
    client.addParams({"verbose": args.verbose})
    # Connect the subcomponent object (SST checks its ELI port list); the
    # latency set here is the one-way delay in each direction.
    sst.Link(f"link{i}").connect((client, "link", args.latency),
                                 (accel, f"client{i}", args.latency))
