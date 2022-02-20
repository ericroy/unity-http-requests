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

common_args=(
    -fvisibility=hidden
    -fno-objc-arc
    -Weverything
    -Wno-objc-missing-property-synthesis
    -pedantic
    "$build_type"
    -dynamiclib
    -I./uhr/include
    -I./uhr/objc/src
    -framework Foundation
    ./uhr/objc/src/*.m
)

iphone_archs=(
    armv7
    armv7s
    arm64
)

simulator_archs=(
    i386
)

all_archs=(
    "${iphone_archs[@]}"
    "${simulator_archs[@]}"
)

# Make simulator dylibs
echo "Building simulator dylib"
sysroot=$(xcrun --sdk iphonesimulator --show-sdk-path)
echo "Sysroot: $sysroot"
for arch in "${simulator_archs[@]}"; do
    echo "Building $arch (simulator)"
    clang++ "${common_args[@]}" -isysroot "$sysroot" \
        -mios-simulator-version-min=7.0 \
        -arch "$arch" \
        -o ".build/uhr/uhr-${arch}.dylib"
done

# Make iphone dylibs
echo "Building iphone dylib"
sysroot=$(xcrun --sdk iphoneos --show-sdk-path)
echo "Sysroot: $sysroot"
for arch in "${iphone_archs[@]}"; do
    echo "Building $arch (iphoneos)"
    clang++ "${common_args[@]}" -isysroot "$sysroot" \
        -miphoneos-version-min=7.0 \
        -arch "$arch" \
        -o ".build/uhr/uhr-${arch}.dylib"
done

# Combine into uber fat dylib
echo "Lipo fat dylib"
lipo_args=()
for arch in "${all_archs[@]}"; do
    lipo_args+=(-arch "$arch" ".build/uhr/uhr-${arch}.dylib")
done
lipo -create "${lipo_args[@]}" -output .build/uhr/uhr.dylib

# Install to plugins directory
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
