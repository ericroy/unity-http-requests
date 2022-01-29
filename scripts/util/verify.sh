#!/bin/bash -e
file=$1
expected=$2

IFS=" " read -r -a sha1_output <<< "$(sha1sum "$file")"
actual=${sha1_output[0]}

echo "Verifying $file"
if [[ "$expected" != "$actual" ]]; then
    echo "SHA1 check failed for: $file"
    echo "  Expected: $expected"
    echo "    Actual: $actual"
    exit 1
fi
