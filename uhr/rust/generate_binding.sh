#!/bin/bash -e
here="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
pushd $here

cat ../include/UnityHttpRequests.h | bindgen --ctypes-prefix=cty --use-core > src/binding.rs
