#!/usr/bin/env python3
#
# GeneratePortManifest.py
#
# Copyright (C) 2017-2026 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details
#
# Reads a Verilator "--json-only" AST dump (VTop.tree.json) and emits a
# synthetic port manifest using the same textual convention as the old
# VL_IN*/VL_OUT* macro lines Verilator itself used to emit directly in
# VTop.h for every port:
#
#   VL_IN(&name,msb,lsb);        // scalar/packed-vector port
#   VL_IN(&name[depth],msb,lsb); // unpacked-array port
#
# This lets every existing scripts/Build*.sh script keep parsing ports
# exactly as it always has (grep VL_IN / VL_OUT, split on '(', ',', '['),
# without needing to understand Verilator's C++ port representation
# (VL_IN8/16/32/64/W macros for scalars, VlUnpacked<T,N> for SV unpacked
# arrays, or whatever a future Verilator release emits next). The AST JSON
# is authoritative for name/direction/width/depth regardless of how the
# port ends up declared in VTop.h, so this manifest is generated once from
# the JSON and used in place of VTop.h by every downstream port-discovery
# script.
#
# Unlike VTop.h, this manifest is never compiled -- it is pure text fed to
# grep/sed/awk -- so the exact macro name doesn't matter as long as it
# contains "VL_IN" or "VL_OUT" (and not "VL_INOUT", which every script
# explicitly filters out).

import json
import sys


def build_addr_map(node, out):
    """Recursively index every dict with an 'addr' key by that address."""
    if isinstance(node, dict):
        addr = node.get("addr")
        if addr is not None and addr not in out:
            out[addr] = node
        for v in node.values():
            build_addr_map(v, out)
    elif isinstance(node, list):
        for v in node:
            build_addr_map(v, out)


def resolve_width_depth(dtypep, addr_map):
    """Follow a dtypep reference to (width, depth) for a port's data type."""
    node = addr_map.get(dtypep)
    if node is None:
        raise ValueError(f"unresolved dtypep reference: {dtypep}")

    node_type = node.get("type")
    if node_type == "UNPACKARRAYDTYPE":
        lo, hi = parse_range(node["declRange"])
        depth = hi - lo + 1
        # An unpacked array's own element width comes from its ref type;
        # nested unpacked arrays (arrays of arrays) are not supported by
        # the downstream codegen and are rejected below.
        width, elemDepth = resolve_width_depth(node["refDTypep"], addr_map)
        if elemDepth != 1:
            raise ValueError("nested unpacked arrays are not supported")
        return width, depth

    if node_type == "BASICDTYPE":
        rng = node.get("range")
        if rng is None:
            return 1, 1
        lo, hi = parse_range(rng)
        return hi - lo + 1, 1

    if node_type == "PACKARRAYDTYPE":
        # A packed array dimension folds into the bit width, same as a
        # plain wide vector; recurse into its element type and multiply.
        lo, hi = parse_range(node["declRange"])
        count = hi - lo + 1
        width, elemDepth = resolve_width_depth(node["subDTypep"], addr_map)
        if elemDepth != 1:
            raise ValueError("nested unpacked arrays are not supported")
        return width * count, 1

    raise ValueError(f"unsupported dtype node for a port: {node_type}")


def parse_range(rangeStr):
    """'msb:lsb' or '[msb:lsb]' textual range -> (lo, hi) as ints, lo <= hi."""
    a, b = rangeStr.strip("[]").split(":")
    a, b = int(a), int(b)
    return (a, b) if a <= b else (b, a)


def main():
    if len(sys.argv) != 2:
        print(f"usage: {sys.argv[0]} <VTop.tree.json>", file=sys.stderr)
        return 1

    with open(sys.argv[1]) as f:
        tree = json.load(f)

    addr_map = {}
    build_addr_map(tree, addr_map)

    modules = tree.get("modulesp", [])
    if not modules:
        print("error: no modules found in AST JSON", file=sys.stderr)
        return 1

    lines = []
    for var in modules[0].get("stmtsp", []):
        if var.get("type") != "VAR":
            continue

        direction = var.get("direction")
        # A module port always carries a real direction (INPUT/OUTPUT/
        # INOUT); internal vars, parameters, etc. report direction NONE.
        # isPrimaryIO was tried first as the port marker, but it isn't
        # reliable across Verilator versions: 5.022 and 5.026 report it
        # False for every var, including genuine ports (confirmed against
        # both), while newer Verilator sets it True for ports. direction
        # itself has been consistent across every version tested (5.022,
        # 5.026, 5.052), so filter on that instead. INOUT is intentionally
        # let through here so it still reaches the direction dispatch
        # below and produces the "unsupported direction" error rather than
        # being silently dropped.
        if direction == "NONE":
            continue

        name = var["name"]
        width, depth = resolve_width_depth(var["dtypep"], addr_map)
        msb = width - 1
        lsb = 0
        sigRef = f"&{name}[{depth}]" if depth > 1 else f"&{name}"

        if direction == "INPUT":
            lines.append(f"VL_IN({sigRef},{msb},{lsb});")
        elif direction == "OUTPUT":
            lines.append(f"VL_OUT({sigRef},{msb},{lsb});")
        else:
            print(
                f"error: port '{name}' has unsupported direction '{direction}' "
                "(true bidirectional inout ports are not supported; build with "
                "--pins-inout-enables to split them into input/output pairs)",
                file=sys.stderr,
            )
            return 1

    print("\n".join(lines))
    return 0


if __name__ == "__main__":
    sys.exit(main())
