#!/bin/bash -e
here="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
pushd $here/..

mkdir -p .build .prefix
pushd .build
cmake -G "NMake Makefiles" -DCMAKE_INSTALL_PREFIX=../.prefix ../uhr/cpp
nmake
nmake install
