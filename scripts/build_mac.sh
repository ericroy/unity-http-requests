#!/bin/bash -e
here="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
root="$here/.."
pushd "$root"

# one of: Debug, Release
build_type="${UHR_BUILD_TYPE:-Debug}"

mkdir -p .build/uhr

if [[ "$build_type" == "Release" ]]; then 
    # Optimize
    build_type='-O2'
else
    # Include debug info
    build_type='-g3'
fi

echo "Clang version: $(clang++ --version)"

# Make dylibs
echo "Building fat dylib"
clang++ \
    -fvisibility=hidden \
    -Weverything \
    -pedantic \
    "$build_type" \
    -dynamiclib \
    -arch arm64 \
    -arch x86_64 \
    -I./uhr/include \
    -I./uhr/objc/src \
    -framework Foundation \
    ./uhr/objc/src/*.m \
    -o .build/uhr/uhr.dylib

# Install to plugins directory
artifact=unity/Assets/Plugins/uhr-mac.fat.dylib
mkdir -p "$(dirname "$artifact")"
cp .build/uhr/uhr.dylib "$artifact"

# For the benefit of the ci job log:
echo "Artifact:"
ls -alR "$artifact"
echo "Depends on:"
otool -L "$artifact"
echo "Exports:"
nm -gU "$artifact"
