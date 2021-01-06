#!/bin/bash -e
here="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
pushd $here

if [ -z "${DEBUG}" ]; then 
    debug_level=''
else 
    debug_level='-g3'
fi

out_dir=../../Assets/Plugins/iOS
artifact_basename=UnityHttpRequests-ios-fat
sysroot=$(xcrun --sdk iphoneos --show-sdk-path)

echo "Sysroot is: $sysroot"
mkdir -p $out_dir
clang++ \
    -isysroot $sysroot \
    -arch arm64 \
    -arch armv7 \
    -arch armv7s \
    -miphoneos-version-min=7.0 \
    -fvisibility=hidden \
    -Weverything \
    -pedantic \
    $debug_level \
    -dynamiclib \
    -I../include \
    -framework Foundation \
    -o $out_dir/$artifact_basename.dylib \
    src/Context.m \
	src/HeaderStorage.m \
	src/Result.m \
	src/ResultStorage.m \
	src/UnityHttpRequests.m

# list expoted symbols:
nm -gU $out_dir/$artifact_basename.dylib