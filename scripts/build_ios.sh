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

common_args=(
    -fvisibility=hidden
    -Weverything
    -pedantic
    "$build_type"
    -dynamiclib
    -I./uhr/include
    -I./uhr/objc/src
    -framework Foundation
    ./uhr/objc/src/*.m
)

# Make simulator fat dylib
sysroot=$(xcrun --sdk iphonesimulator --show-sdk-path)
echo "Sysroot is: $sysroot"
clang++ "${common_args[@]}" -isysroot "$sysroot" -mios-simulator-version-min=7.0 \
    -arch i386 \
    -arch x86_64 \
    -o .build/uhr/uhr-iphonesimulator.dylib

# Make iphone fat dylib
sysroot=$(xcrun --sdk iphoneos --show-sdk-path)
echo "Sysroot is: $sysroot"
clang++ "${common_args[@]}" -isysroot "$sysroot" -miphoneos-version-min=7.0 \
    -arch armv7 \
    -arch armv7s \
    -arch arm64 \
    -o .build/uhr/uhr-iphoneos.dylib

# Combine into uber fat dylib
lipo -create \
    -arch i386 .build/uhr/uhr-iphonesimulator.dylib \
    -arch x86_64 .build/uhr/uhr-iphonesimulator.dylib \
    -arch armv7 .build/uhr/uhr-iphoneos.dylib \
    -arch armv7s .build/uhr/uhr-iphoneos.dylib \
    -arch arm64 .build/uhr/uhr-iphoneos.dylib \
    -output .build/uhr/uhr.dylib

# List expoted symbols:
nm -gU .build/uhr/uhr.dylib

artifact=unity/Assets/Plugins/iOS/uhr-ios.fat.dylib
mkdir -p "$(dirname "$artifact")"
cp .build/uhr/uhr.dylib "$artifact"

# For the benefit of the ci job log:
echo "Artifact:"
ls -alR "$artifact"
echo "Depends on:"
otool -L "$artifact"
echo "Exports:"
nm -gU "$artifact"
