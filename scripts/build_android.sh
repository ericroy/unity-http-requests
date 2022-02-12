#!/bin/bash -e
set -e
set -x
here="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
root="$here/.."
pushd "$root"

if [[ "$(arch)" != "x86_64" ]]; then
    echo "Expected host to be x86_64, but it was $(arch)"
    exit 1
fi

: "${UHR_ANDROID_NDK_ROOT:?Path must be provided}"

# one of: Debug, Release
build_type="${UHR_BUILD_TYPE:-Debug}"

# Android api version to target, 24 == Android 7.0 Nougat
target_api_version="${UHR_ANDROID_TARGET_API:-24}"

arch_abi="${UHR_ANDROID_ARCH:-armeabi-v7a}"

mkdir -p .build .prefix

# Pull down third party dependencies (curl, mbedtls, etc)
./scripts/util/fetch_deps.sh

common_args=(
    -DCMAKE_BUILD_TYPE="$build_type"
    -DCMAKE_FIND_DEBUG_MODE:BOOL=true
    -DCMAKE_PREFIX_PATH="$root/.prefix"
    -DCMAKE_INSTALL_PREFIX="$root/.prefix"
    -DCMAKE_MODULE_PATH="../../CMake"
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=true
    -DANDROID_ABI="$arch_abi"
    -DANDROID_NDK="$UHR_ANDROID_NDK_ROOT"
    -DANDROID_PLATFORM="android-$target_api_version"
    -DCMAKE_SYSTEM_NAME=Android
    -DCMAKE_SYSTEM_VERSION="$target_api_version"
    -DCMAKE_ANDROID_ARCH_ABI="$arch_abi"
    -DCMAKE_ANDROID_NDK="$UHR_ANDROID_NDK_ROOT"
    -DCMAKE_TOOLCHAIN_FILE="$UHR_ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake"
)

# utfcpp
mkdir -p .build/utfcpp
pushd .build/utfcpp
cmake "${common_args[@]}" -DUTF8_TESTS:BOOL=false -DUTF8_SAMPLES:BOOL=false -DUTF8_INSTALL:BOOL=true ../../uhr/cpp/deps/utfcpp
make "-j$(nproc)" && make install
popd

# zlib
mkdir -p .build/zlib
pushd .build/zlib
cmake "${common_args[@]}" -DBUILD_SHARED_LIBS:BOOL=false ../../uhr/cpp/deps/zlib
make "-j$(nproc)" && make install
popd


# mbedtls
mkdir -p .build/mbedtls
pushd .build/mbedtls
cmake "${common_args[@]}" -DENABLE_TESTING:BOOL=false -DENABLE_PROGRAMS:BOOL=false ../../uhr/cpp/deps/mbedtls
make "-j$(nproc)" && make install
popd

# curl
# USE_ZLIB=true means build curl with features that rely on zlib.
# CURL_ZLIB="" means don't call find_package for zlib.  We will be responsible
# for making sure that zlib functions are available in the final binary,
# which we'll achieve by statically linking zlib ourselves.
mkdir -p .build/curl
pushd .build/curl
cmake "${common_args[@]}" \
    -DBUILD_SHARED_LIBS:BOOL=false \
    -DBUILD_CURL_EXE:BOOL=false \
    -DBUILD_TESTING:BOOL=false \
    -DHTTP_ONLY:BOOL=true \
    -DCURL_ZLIB=ON \
    -DCMAKE_USE_LIBSSH2:BOOL=false \
    -DCMAKE_USE_OPENSSL:BOOL=false \
    -DCMAKE_USE_SCHANNEL:BOOL=false \
    -DCMAKE_USE_MBEDTLS:BOOL=true \
    -DHAVE_POLL_FINE_EXITCODE:BOOL=false \
    -DHAVE_POLL_FINE_EXITCODE__TRYRUN_OUTPUT="" \
    ../../uhr/cpp/deps/curl
make "-j$(nproc)" && make install
popd

# uhr
mkdir -p .build/uhr
pushd .build/uhr
cmake "${common_args[@]}" ../../uhr/cpp
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