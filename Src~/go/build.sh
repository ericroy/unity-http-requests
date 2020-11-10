#!/bin/bash -e
here="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"

pushd $here

# Editor/Win64
go build -ldflags="-s -w" -buildmode c-shared -o ../../Assets/Plugins/Editor/UnityHttpRequests.dll ./cmd/unity_http_requests/

# iOS
# To build a cross compiling toolchain for iOS on OS X, first modify clangwrap.sh in misc/ios to match your setup. And then run:
# GOARM=7 CGO_ENABLED=1 GOARCH=arm CC_FOR_TARGET=`pwd`/../misc/ios/clangwrap.sh CXX_FOR_TARGET=`pwd`/../misc/ios/clangwrap.sh ./make.bash
# Build: GOARCH=arm go build -buildmode c-shared -o $here/../../Assets/Plugins/iOS/ $here/cmd/unity_http_requests/

# Android
# Get NDK somehow: https://developer.android.com/ndk/downloads/index.html
# Install toolchain: ./android-ndk-r10c/build/tools/make-standalone-toolchain.sh --platform=android-21 --install-dir=$NDK_ROOT
# Build: CC="$NDK_ROOT/bin/arm-linux-androideabi-gcc" GOOS=linux GOARCH=arm GOARM=7 go build -buildmode c-shared -o $here/../../Assets/Plugins/Android/ $here/cmd/unity_http_requests/