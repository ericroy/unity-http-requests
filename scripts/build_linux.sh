#!/bin/bash -e
here="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
pushd "$here/.."

if [[ "$(arch)" != "x86_64" ]]; then
    echo "Expected host to be x86_64, but it was $(arch)"
    exit 1
fi

# one of: Debug, Release
build_type="${UHR_BUILD_TYPE:-Debug}"

mkdir -p .build .prefix

# Pull down third party dependencies (curl, mbedtls, etc)
./scripts/util/fetch_deps.sh

read -r cmake_common_args <<EOF
    -DCMAKE_BUILD_TYPE=$build_type \
    -DCMAKE_FIND_DEBUG_MODE:BOOL=true \
    -DCMAKE_PREFIX_PATH=$(pwd)/../../.prefix \
    -DCMAKE_INSTALL_PREFIX=../../.prefix \
    -DCMAKE_MODULE_PATH=$(pwd)/CMake \
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=true
)
EOF

# utfcpp
mkdir -p .build/utfcpp
pushd .build/utfcpp
cmake $cmake_common_args -DUTF8_TESTS:BOOL=false -DUTF8_SAMPLES:BOOL=false -DUTF8_INSTALL:BOOL=true ../../uhr/cpp/deps/utfcpp
make "-j$(nproc)" && make install
popd

# zlib
mkdir -p .build/zlib
pushd .build/zlib
cmake $cmake_common_args -DBUILD_SHARED_LIBS:BOOL=false ../../uhr/cpp/deps/zlib
make "-j$(nproc)" && make install
popd

# mbedtls
mkdir -p .build/mbedtls
pushd .build/mbedtls
cmake $cmake_common_args -DENABLE_TESTING:BOOL=false -DENABLE_PROGRAMS:BOOL=false ../../uhr/cpp/deps/mbedtls
make "-j$(nproc)" && make install
popd

# curl
mkdir -p .build/curl
pushd .build/curl
cmake $cmake_common_args \
    -DBUILD_SHARED_LIBS:BOOL=false \
    -DBUILD_CURL_EXE:BOOL=false \
    -DBUILD_TESTING:BOOL=false \
    -DHTTP_ONLY:BOOL=true \
    -DCURL_ZLIB=ON \
    -DCMAKE_USE_LIBSSH2:BOOL=false \
    -DCMAKE_USE_OPENSSL:BOOL=false \
    -DCMAKE_USE_SCHANNEL:BOOL=false \
    -DCMAKE_USE_MBEDTLS:BOOL=true \
    ../../uhr/cpp/deps/curl
make "-j$(nproc)" && make install
popd

# uhr
mkdir -p .build/uhr
pushd .build/uhr
cmake $cmake_common_args ../../uhr/cpp
make "-j$(nproc)" && make install
popd

artifact=unity/Assets/Plugins/x86_64/uhr-linux.x86_64.so
mkdir -p "$(dirname "$artifact")"
cp .prefix/lib/libuhr.so $artifact

# For the benefit of the ci job log:
echo "Artifact:"
ls -alR $artifact
echo "Depends on:"
readelf -d $artifact | grep NEEDED
echo "Exports:"
readelf -s "$artifact" | while read -r num _ _ type bind _ index name _ ; do
    [ "$type" = "FUNC" ] || continue
    [ "$bind" = "GLOBAL" ] || continue
    [ "${num::-1}" = "$((${num::-1}))" ] || continue
    [ "$index" = "$((index))" ] || continue
    printf '\t%s\n' "$name"
done