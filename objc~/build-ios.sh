#!/bin/bash -e
here="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
pushd $here

sysroot=$(xcrun --sdk iphoneos --show-sdk-path)
echo "Sysroot is: $sysroot"

clang++ \
    -isysroot $sysroot \
    -arch arm64 \
    -arch armv7 \
    -arch armv7s \
    -miphoneos-version-min=7.0 \
    -dynamiclib \
    -o ../Assets/iOS/UnityHttpRequests.dylib \
    src/Context.m \
	src/HeaderStorage.m \
	src/Result.m \
	src/ResultStorage.m \
	src/UnityHttpRequests.m