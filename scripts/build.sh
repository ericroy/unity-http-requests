#!/bin/bash -e
here="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
pushd $here/..

mkdir -p .build .prefix

# If UHR_NDK_ROOT is set, we're building for android
cmake_extra=""
if [[ ! -z "$UHR_NDK_ROOT" ]]; then
    echo "Building for android"
    cmake_extra="-DCMAKE_ANDROID_NDK=${UHR_NDK_ROOT}"
elif [ "$OSTYPE" = "msys" ]; then
    echo "Building for windows"
else
    echo "Building for linux"
fi

# Pull down third party dependencies (curl, mbedtls, etc)
source scripts/fetch_deps.sh

# mbedtls
mkdir -p .build/mbedtls
pushd .build/mbedtls
cmake -DCMAKE_INSTALL_PREFIX=../../.prefix \
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=true \
    -DENABLE_TESTING:BOOL=false \
	-DENABLE_PROGRAMS:BOOL=false \
    $cmake_extra \
    ../../uhr/cpp/deps/mbedtls
make -j$(nproc) && make install
popd

# curl
mkdir -p .build/curl
pushd .build/curl
if [ "$OSTYPE" = "msys" ]; then
    ssl_backed="-DCMAKE_USE_MBEDTLS:BOOL=false -DCMAKE_USE_SCHANNEL:BOOL=true"
else
    ssl_backed="-DCMAKE_USE_MBEDTLS:BOOL=true -DCMAKE_USE_SCHANNEL:BOOL=false"
fi
cmake -DCMAKE_INSTALL_PREFIX=../../.prefix \
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=true \
    -DBUILD_SHARED_LIBS:BOOL=false \
    -DBUILD_CURL_EXE:BOOL=false \
    -DBUILD_TESTING:BOOL=false \
    -DHTTP_ONLY:BOOL=true \
    -DCMAKE_USE_LIBSSH2:BOOL=false \
    -DCMAKE_USE_OPENSSL:BOOL=false \
    $ssl_backed \
    $cmake_extra \
    ../../uhr/cpp/deps/curl
make -j$(nproc) && make install
popd

# uhr
mkdir -p .build/uhr
pushd .build/uhr
cmake -DCMAKE_INSTALL_PREFIX=../../.prefix \
    $cmake_extra \
    ../../uhr/cpp
make -j$(nproc) && make install
popd
