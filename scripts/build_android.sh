#!/bin/bash -e
here="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
pushd "$here/.."

if [[ "$(arch)" != "x86_64" ]]; then
    echo "Expected host to be x86_64, but it was $(arch)"
    exit 1
fi

# one of: Debug, Release
build_type="${UHR_BUILD_TYPE:-Debug}"

# Android api version to target, 24 == Android7.0
target_api_version="${UHR_ANDROID_TARGET_API:-24}"

android_ndk_root="${UHR_ANDROID_NDK_ROOT:-}"

mkdir -p .build .prefix

# Pull down third party dependencies (curl, mbedtls, etc)
./scripts/util/fetch_deps.sh

IFS='' read -r -d '' cmake_common_android_args <<EOF
-DCMAKE_SYSTEM_NAME=Android \
-DCMAKE_ANDROID_API="$target_api_version" \
-DCMAKE_ANDROID_ARCH_ABI=armeabi-v7a \
-DCMAKE_ANDROID_NDK="$android_ndk_root" \
EOF

# utfcpp
mkdir -p .build/utfcpp
pushd .build/utfcpp
cmake -DCMAKE_BUILD_TYPE="$build_type" \
    -DCMAKE_INSTALL_PREFIX=../../.prefix \
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=true \
    -DUTF8_TESTS=false \
    -DUTF8_SAMPLES=false \
    -DUTF8_INSTALL=true \
    "$cmake_common_android_args" \
    ../../uhr/cpp/deps/utfcpp
make "-j$(nproc)" && make install
popd

# zlib
mkdir -p .build/zlib
pushd .build/zlib
cmake -DCMAKE_BUILD_TYPE="$build_type" \
    -DCMAKE_INSTALL_PREFIX=../../.prefix \
    -DBUILD_SHARED_LIBS:BOOL=false \
    "$cmake_common_android_args" \
    ../../uhr/cpp/deps/zlib
make "-j$(nproc)" && make install
popd

# mbedtls
mkdir -p .build/mbedtls
pushd .build/mbedtls
cmake -DCMAKE_BUILD_TYPE="$build_type" \
    -DCMAKE_INSTALL_PREFIX=../../.prefix \
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=true \
    -DENABLE_TESTING:BOOL=false \
    -DENABLE_PROGRAMS:BOOL=false \
    "$cmake_common_android_args" \
    ../../uhr/cpp/deps/mbedtls
make "-j$(nproc)" && make install
popd

# curl
# USE_ZLIB=true means build curl with features that rely on zlib.
# CURL_ZLIB="" means don't call find_package for zlib.  We will be responsible
# for making sure that zlib functions are available in the final binary,
# which we'll achieve by statically linking zlib ourselves.
mkdir -p .build/curl
pushd .build/curl
cmake -DCMAKE_BUILD_TYPE="$build_type" \
    -DCMAKE_INSTALL_PREFIX=../../.prefix \
    -DZLIB_ROOT=../../.prefix \
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=true \
    -DCURL_HIDDEN_SYMBOLS:BOOL=true \
    -DBUILD_SHARED_LIBS:BOOL=false \
    -DBUILD_CURL_EXE:BOOL=false \
    -DBUILD_TESTING:BOOL=false \
    -DHTTP_ONLY:BOOL=true \
    -DCMAKE_USE_LIBSSH2:BOOL=false \
    -DCMAKE_USE_OPENSSL:BOOL=false \
    -DCMAKE_USE_MBEDTLS:BOOL=true \
    -DCMAKE_USE_SCHANNEL:BOOL=false \
    -DCMAKE_USE_ZLIB:BOOL=true \
    -DCMAKE_CURL_ZLIB="" \
    -DHAVE_POLL_FINE_EXITCODE:BOOL=false \
    -DHAVE_POLL_FINE_EXITCODE__TRYRUN_OUTPUT="" \
    "$cmake_common_android_args" \
    ../../uhr/cpp/deps/curl
make "-j$(nproc)" && make install
popd

# uhr
mkdir -p .build/uhr
pushd .build/uhr
cmake -DCMAKE_BUILD_TYPE="$build_type" \
    -DCMAKE_INSTALL_PREFIX=../../.prefix \
    "$cmake_common_android_args" \
    ../../uhr/cpp
make "-j$(nproc)" && make install
popd


artifact=unity/Assets/Plugins/Android/uhr-android.armeabi-v7a.so
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