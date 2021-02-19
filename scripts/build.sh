#!/bin/bash -e
here="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
pushd $here/..

mkdir -p .build .prefix

cmake_extra=""
generator="Unix Makefiles"
make_cmd="make -j$(nproc)"
use_mbedtls="true"
use_schannel="false"

if [ "$OSTYPE" = "msys" ]; then
	generator="NMake Makefiles"
	make_cmd="nmake"
	use_mbedtls="false"
	use_schannel="true"
fi

# If UHR_NDK_ROOT is set, we're building for android
if [[ ! -z "$UHR_NDK_ROOT" ]]; then
	echo "Building for android"
	cmake_extra="$cmake_extra -DCMAKE_ANDROID_NDK=${UHR_NDK_ROOT}"
fi

# Pull down third party dependencies (curl, mbedtls, etc)
source scripts/fetch_deps.sh

# mbedtls
mkdir -p .build/mbedtls
cmake -G "$generator" \
	-B .build/mbedtls \
	-DCMAKE_INSTALL_PREFIX=.prefix \
	-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=true \
	-DENABLE_TESTING:BOOL=false \
	-DENABLE_PROGRAMS:BOOL=false \
	$cmake_extra \
	uhr/cpp/deps/mbedtls

# curl
mkdir -p .build/curl
cmake -G "$generator" \
	-B .build/curl \
	-DCMAKE_INSTALL_PREFIX=.prefix \
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
	uhr/cpp/deps/curl

# uhr
mkdir -p .build/uhr
cmake -G "$generator" \
	-B .build/uhr \
	-DCMAKE_INSTALL_PREFIX=.prefix \
	$cmake_extra \
	uhr/cpp

# Build all
projects=(.build/mbedtls .build/curl .build/uhr)
for project_dir in ${projects[*]} ; do
	pushd $project_dir
	$make_cmd && $make_cmd install
	popd
done
