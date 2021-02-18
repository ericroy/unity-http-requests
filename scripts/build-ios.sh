#!/bin/bash -e
here="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
pushd $here/..

mkdir -p .build .prefix

if [ -z "${DEBUG}" ]; then 
    debug_level=''
else 
    debug_level='-g3'
fi

artifact=UnityHttpRequests-ios-fat.dylib
sysroot=$(xcrun --sdk iphoneos --show-sdk-path)

echo "Sysroot is: $sysroot"
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
    -I./uhr/include \
    -framework Foundation \
    -o .build/$artifact \
    ./uhr/objc/src/Context.m \
	./uhr/objc/src/HeaderStorage.m \
	./uhr/objc/src/Result.m \
	./uhr/objc/src/ResultStorage.m \
	./uhr/objc/src/UnityHttpRequests.m

# list expoted symbols:
nm -gU .build/$artifact

cp .build/$artifact .prefix/
