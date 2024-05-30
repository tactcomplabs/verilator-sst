#!/bin/bash
# Usage: ./check_missing_symbols.sh <path_to_shared_library>

LIB_PATH=$1

if [[ ! -f $LIB_PATH ]]; then
    echo "File not found: $LIB_PATH"
    exit 1
fi

# Using nm to list symbols, and awk to filter out undefined ones
nm -D $LIB_PATH | awk '$1=="U" { print }'
