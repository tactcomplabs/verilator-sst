#!/bin/bash
# GeneratePortManifest.sh
#
# Copyright (C) 2017-2026 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
# See LICENSE in the top level directory for licensing details
#
# Emits a port manifest using the same textual convention every
# scripts/Build*.sh script already parses (grep VL_IN / VL_OUT, split on
# '(', ',', '['): one VL_IN(&name[depth],msb,lsb); / VL_OUT(...) line per
# port. Scalar and packed-vector ports are still declared in VTop.h as
# VL_IN8/16/32/64/W macros, so those lines pass straight through.
#
# SV *unpacked-array* ports (e.g. `output [31:0] accum[4]`) are declared in
# VTop.h as `VlUnpacked<T/*msb:lsb*/, N> &name;` instead, which carries width
# (the /*msb:lsb*/ comment) and depth (the template's second argument) but,
# unlike the VL_IN*/VL_OUT* macros, no direction at all. Direction for those
# is recovered with a best-effort grep of the original Verilog/SystemVerilog
# source for an "input"/"output" declaration naming that same signal. This
# is the dependency-free tradeoff: no JSON/AST parsing, just text -- but it
# only covers the common one-signal-per-declaration-line style (see
# resolve_direction below) and falls back to a hard error rather than
# silently guessing when a source's style isn't recognized.

set -e
VTOP=$1
SRC=$2

if [[ -z "$VTOP" || -z "$SRC" ]]; then
  echo "usage: $0 <VTop.h> <sv-source-path-or-glob>" >&2
  exit 1
fi

# $SRC may be a glob (e.g. ".../uart_mem/*.sv"), same as BuildVerilatorSrc.sh
# receives it -- leave it unquoted here so bash expands it, same as there.
SOURCES=($SRC)

# Pass scalar/packed-vector ports straight through.
grep "VL_IN\|VL_OUT" "$VTOP" || true

resolve_direction() {
  local name=$1
  local hit
  hit=$(grep -hE "^[[:space:]]*(input|output)\b.*[^a-zA-Z0-9_]${name}[[:space:]]*(\[|,|;|\))" "${SOURCES[@]}" | head -1)
  if [[ -z "$hit" ]]; then
    echo "error: could not find an input/output declaration for port '${name}' in: ${SOURCES[*]}" >&2
    exit 1
  fi
  if [[ "$hit" =~ ^[[:space:]]*input ]]; then
    echo "INPUT"
  elif [[ "$hit" =~ ^[[:space:]]*output ]]; then
    echo "OUTPUT"
  else
    echo "error: unrecognized declaration for port '${name}': ${hit}" >&2
    exit 1
  fi
}

# Unpacked-array ports: VlUnpacked<TYPE/*msb:lsb*/, depth> &name;
grep "VlUnpacked" "$VTOP" | while read -r line; do
  msb=$(echo "$line" | sed -n 's/.*\/\*\([0-9]*\):\([0-9]*\)\*\/.*/\1/p')
  lsb=$(echo "$line" | sed -n 's/.*\/\*\([0-9]*\):\([0-9]*\)\*\/.*/\2/p')
  depth=$(echo "$line" | sed -n 's/.*,[[:space:]]*\([0-9]*\)[[:space:]]*>.*/\1/p')
  name=$(echo "$line" | sed -n 's/.*&\([A-Za-z_][A-Za-z0-9_]*\);.*/\1/p')

  if [[ -z "$msb" || -z "$lsb" || -z "$depth" || -z "$name" ]]; then
    echo "error: could not parse VlUnpacked port declaration: ${line}" >&2
    exit 1
  fi

  direction=$(resolve_direction "$name")
  if [[ "$direction" == "INPUT" ]]; then
    echo "VL_IN(&${name}[${depth}],${msb},${lsb});"
  else
    echo "VL_OUT(&${name}[${depth}],${msb},${lsb});"
  fi
done

# -- EOF
