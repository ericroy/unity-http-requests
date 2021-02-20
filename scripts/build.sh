#!/bin/bash -e
here="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
pushd $here/..

arch=$(arch)
platform=unknown
if [[ "$OSTYPE" = "msys" ]]; then
	platform=windows
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
	platform=linux
else
	echo "Unhandled OS case ($OSTYPE)"
	exit 1
fi

# one of: Debug, Release
build_type="${UHR_BUILD_TYPE:-Debug}"

# one of: linux, windows, mac, ios, android
target_platform="${UHR_TARGET_PLATFORM:-$platform}"

# one of: x86_64, armeabi-v7a, arm64-v8a
target_arch="${UHR_TARGET_ARCH:-$arch}"

android_ndk_root="${UHR_ANDROID_NDK_ROOT:-}"

echo "Host   : platform=$platform, arch=$arch"
echo "Target : platform=$target_platform, arch=$target_arch"
echo "Config : build_type=$build_type"

cmake_extra="-DCMAKE_BUILD_TYPE=$build_type"
generator="Unix Makefiles"
make_cmd="make -j$(nproc)"
use_mbedtls="true"
use_schannel="false"

if [[ "$platform" = "windows" ]]; then
	if jom -version ; then
		generator="NMake Makefiles JOM"
		make_cmd="jom"
	else
		generator="NMake Makefiles"
		make_cmd="nmake"
	fi
fi

if [[ "$target_platform" = "windows" ]]; then
	use_mbedtls="false"
	use_schannel="true"
fi

if [[ "$target_platform" = "android" ]]; then
	cmake_extra="$cmake_extra -DCMAKE_ANDROID_NDK=${android_ndk_root} -DCMAKE_SYSTEM_NAME=Android -DCMAKE_ANDROID_ARCH_ABI=$target_arch"
fi


mkdir -p .build .prefix

# Pull down third party dependencies (curl, mbedtls, etc)
./scripts/util/fetch_deps.sh

# mbedtls
mkdir -p .build/mbedtls
pushd .build/mbedtls
cmake -G "$generator" \
	-DCMAKE_INSTALL_PREFIX=../../.prefix \
	-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=true \
	-DENABLE_TESTING:BOOL=false \
	-DENABLE_PROGRAMS:BOOL=false \
	$cmake_extra \
	../../uhr/cpp/deps/mbedtls
$make_cmd && $make_cmd install
popd

# curl
mkdir -p .build/curl
pushd .build/curl
cmake -G "$generator" \
	-DCMAKE_INSTALL_PREFIX=../../.prefix \
	-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=true \
	-DBUILD_SHARED_LIBS:BOOL=false \
	-DBUILD_CURL_EXE:BOOL=false \
	-DBUILD_TESTING:BOOL=false \
	-DHTTP_ONLY:BOOL=true \
	-DCMAKE_USE_LIBSSH2:BOOL=false \
	-DCMAKE_USE_OPENSSL:BOOL=false \
	-DCMAKE_USE_MBEDTLS:BOOL=$use_mbedtls \
	-DCMAKE_USE_SCHANNEL:BOOL=$use_schannel \
	$cmake_extra \
	../../uhr/cpp/deps/curl
$make_cmd && $make_cmd install
popd

# uhr
mkdir -p .build/uhr
pushd .build/uhr
cmake -G "$generator" \
	-DCMAKE_INSTALL_PREFIX=../../.prefix \
	$cmake_extra \
	../../uhr/cpp
$make_cmd && $make_cmd install
popd

if [[ "$target_platform" = "windows" ]]; then
	mkdir -p unity/Assets/Plugins/x86_64
	cp .prefix/bin/uhr.dll unity/Assets/Plugins/x86_64/uhr-windows.$target_arch.dll
elif [[ "$target_platform" = "linux" ]]; then
	mkdir -p unity/Assets/Plugins/x86_64
	cp .prefix/lib/libuhr.so unity/Assets/Plugins/x86_64/uhr-linux.$target_arch.so
elif [[ "$target_platform" = "android" ]]; then
	mkdir -p unity/Assets/Plugins/Android
	cp .prefix/lib/libuhr.so unity/Assets/Plugins/Android/uhr-android.$target_arch.so
else
	echo "Unhandled target_platform case ($target_platform)"
	exit 1
fi