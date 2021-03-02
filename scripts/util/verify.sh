#!/bin/bash
set -euo pipefail
file=$1
expected=$2
actual=(`sha1sum $file`)
echo "Verifying $file"
if [[ "$expected" != "$actual" ]]; then
    echo "SHA1 check failed for: $file"
    echo "  Expected: $expected"
    echo "    Actual: $actual"
    exit 1
fi
