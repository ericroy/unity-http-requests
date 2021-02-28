#!/bin/bash -e
here="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
pushd $here/..

if [[ "$(arch)" != "x86_64" ]]; then
    echo "Expected host to be x86_64, but it was $(arch)"
    exit 1
fi

# one of: Debug, Release
build_type="${UHR_BUILD_TYPE:-Debug}"

android_ndk_root="${UHR_ANDROID_NDK_ROOT:-}"

mkdir -p .build .prefix

# Pull down third party dependencies (curl, mbedtls, etc)
./scripts/util/fetch_deps.sh

# mbedtls
mkdir -p .build/mbedtls
pushd .build/mbedtls
cmake -DCMAKE_BUILD_TYPE=$build_type \
    -DCMAKE_INSTALL_PREFIX=../../.prefix \
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=true \
    -DENABLE_TESTING:BOOL=false \
    -DENABLE_PROGRAMS:BOOL=false \
    -DCMAKE_SYSTEM_NAME=Android \
    -DCMAKE_ANDROID_ARCH_ABI=armeabi-v7a \
    -DCMAKE_ANDROID_NDK=$android_ndk_root \
    ../../uhr/cpp/deps/mbedtls
make -j$(nproc) && make install
popd

# curl
mkdir -p .build/curl
pushd .build/curl
cmake -DCMAKE_BUILD_TYPE=$build_type \
	-DCMAKE_INSTALL_PREFIX=../../.prefix \
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
    -DCMAKE_SYSTEM_NAME=Android \
    -DCMAKE_ANDROID_ARCH_ABI=armeabi-v7a \
    -DCMAKE_ANDROID_NDK=$android_ndk_root \
    -DHAVE_POLL_FINE_EXITCODE:BOOL=false \
    -DHAVE_POLL_FINE_EXITCODE__TRYRUN_OUTPUT="" \
	../../uhr/cpp/deps/curl
make -j$(nproc) && make install
popd

# uhr
mkdir -p .build/uhr
pushd .build/uhr
cmake -DCMAKE_BUILD_TYPE=$build_type \
	-DCMAKE_INSTALL_PREFIX=../../.prefix \
    -DCMAKE_SYSTEM_NAME=Android \
    -DCMAKE_ANDROID_ARCH_ABI=armeabi-v7a \
    -DCMAKE_ANDROID_NDK=$android_ndk_root \
	../../uhr/cpp
make -j$(nproc) && make install
popd


artifact=unity/Assets/Plugins/Android/uhr-android.armeabi-v7a.so
mkdir -p `dirname $artifact`
cp .prefix/lib/libuhr.so $artifact

# For the benefit of the ci job log:
echo "Artifact:"
ls -alR $artifact
echo "Depends on:"
readelf -d $artifact | grep NEEDED
echo "Exports:"
readelf -s "$artifact" | while read num value size type bind viz index name dummy ; do
	[ "$type" = "FUNC" ] || continue
	[ "$bind" = "GLOBAL" ] || continue
	[ "${num::-1}" = "$[${num::-1}]" ] || continue
	[ "$index" = "$[$index]" ] || continue
	printf '\t%s\n' "$name"
done