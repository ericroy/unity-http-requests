#!/bin/bash -e
here="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
pushd $here/..

mkdir -p .build .prefix
pushd .build
cmake -DCMAKE_INSTALL_PREFIX=../.prefix ../uhr/cpp
make -j$(nproc)
make install
