#!/bin/bash -e
here="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
pushd $here

if [ -z "${DEBUG}" ]; then 
    debug_level=''
else 
    debug_level='-g3'
fi

sources=\
    uhr/objc/src/Context.m \
	uhr/objc/src/HeaderStorage.m \
	uhr/objc/src/ResultStorage.m \
	uhr/objc/src/UnityHttpRequests.m

if [ -z "${DOCKER}" ]; then
    # Real macos, not Docker/ubuntu
    out_dir=./Assets/Plugins/iOS
    artifact_basename=UnityHttpRequests
    sysroot=$(xcrun --sdk iphoneos --show-sdk-path)
    
    echo "Sysroot is: $sysroot"
    mkdir -p $out_dir
    clang++ \
        $debug_level \
        -isysroot $sysroot \
        -arch arm64 \
        -arch armv7 \
        -arch armv7s \
        -miphoneos-version-min=7.0 \
        -fvisibility=hidden \
        -Weverything \
        -pedantic \
        -dynamiclib \
        -I./include \
        -framework Foundation \
        -o $out_dir/$artifact_basename.dylib \
        $sources
else
    # Docker/ubuntu
    out_dir=.
    artifact_basename=UnityHttpRequests-gnustep
    gnustep_extras="$(gnustep-config --objc-flags) $(gnustep-config --objc-libs) -lgnustep-base"

    mkdir -p $out_dir
    clang++ \
        $gnustep_extras \
        $debug_level \
        -fvisibility=hidden \
        -Weverything \
        -pedantic \
        -dynamiclib \
        -I./include \
        -framework Foundation \
        -o $out_dir/$artifact_basename.dylib \
        $sources
fi

# list expoted symbols:
nm -gU $out_dir/$artifact_basename.dylib