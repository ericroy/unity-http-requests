#!/bin/bash -e
here="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
pushd $here/..

# one of: Debug, Release
build_type="${UHR_BUILD_TYPE:-Debug}"

mkdir -p .build/uhr

if [[ "$build_type" = "Release" ]]; then 
    # Optimize
    build_type='-O2'
else
    # Include debug info
    build_type='-g3'
fi

sysroot=$(xcrun --sdk iphoneos --show-sdk-path)

echo "Sysroot is: $sysroot"
clang++ \
    -isysroot $sysroot \
    -arch i386 \
    -arch arm64 \
    -arch armv7 \
    -arch armv7s \
    -miphoneos-version-min=7.0 \
    -fvisibility=hidden \
    -Weverything \
    -pedantic \
    $build_type \
    -dynamiclib \
    -I./uhr/include \
    -I./uhr/objc/src \
    -framework Foundation \
    -o .build/uhr/uhr.dylib \
    ./uhr/objc/src/*.m

# list expoted symbols:
nm -gU .build/uhr/uhr.dylib

mkdir -p unity/Assets/Plugins/iOS
cp .build/uhr/uhr.dylib unity/Assets/Plugins/iOS/uhr-ios.fat.dylib
