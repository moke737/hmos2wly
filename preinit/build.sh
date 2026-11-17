#!/bin/bash
# Build hmos2negroni preinit (static aarch64) from the workspace source
set -uo pipefail
W="/mnt/e/1+10pro/1+10pro[HMOS]"
B=/root/m0b
mkdir -p "$B"
aarch64-linux-gnu-gcc -O2 -static -Wall -Wno-format-truncation \
    -o "$B/preinit" "$W/m0b/preinit/preinit.c"
rc=$?
echo "gcc rc=$rc"
ls -l "$B/preinit" 2>/dev/null
file "$B/preinit" 2>/dev/null
exit $rc
