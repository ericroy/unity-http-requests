#!/bin/bash -e
here="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
pushd $here

sysroot=$(xcrun --sdk iphoneos --show-sdk-path)
echo "Sysroot is: $sysroot"

artifact_basename=UnityHttpRequests-ios-fat

clang++ \
    -isysroot $sysroot \
    -arch arm64 \
    -arch armv7 \
    -arch armv7s \
    -miphoneos-version-min=7.0 \
    -dynamiclib \
    -o ../Assets/Plugins/iOS/$artifact_basename.dylib \
    -framework Foundation \
    src/Context.m \
	src/HeaderStorage.m \
	src/Result.m \
	src/ResultStorage.m \
	src/UnityHttpRequests.m
