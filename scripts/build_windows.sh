#!/bin/bash -e
here="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
pushd $here/..

if [[ "$(arch)" != "x86_64" ]]; then
    echo "Expected host to be x86_64, but it was $(arch)"
    exit 1
fi

# one of: Debug, Release
build_type="${UHR_BUILD_TYPE:-Debug}"

mkdir -p .build .prefix

# Pull down third party dependencies (curl, mbedtls, etc)
./scripts/util/fetch_deps.sh

if jom -version ; then
    generator="NMake Makefiles JOM"
    make_cmd="jom"
else
    generator="NMake Makefiles"
    make_cmd="nmake"
fi

# curl
mkdir -p .build/curl
pushd .build/curl
cmake -G "$generator" \
    -DCMAKE_BUILD_TYPE=$build_type \
	-DCMAKE_INSTALL_PREFIX=../../.prefix \
    -DCURL_STATIC_CRT:BOOL=true \
	-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=true \
	-DBUILD_SHARED_LIBS:BOOL=false \
	-DBUILD_CURL_EXE:BOOL=false \
	-DBUILD_TESTING:BOOL=false \
	-DHTTP_ONLY:BOOL=true \
	-DCMAKE_USE_LIBSSH2:BOOL=false \
	-DCMAKE_USE_OPENSSL:BOOL=false \
	-DCMAKE_USE_MBEDTLS:BOOL=false \
	-DCMAKE_USE_SCHANNEL:BOOL=true \
	../../uhr/cpp/deps/curl
$make_cmd && $make_cmd install
popd

# uhr
mkdir -p .build/uhr
pushd .build/uhr
cmake -G "$generator" \
    -DCMAKE_BUILD_TYPE=$build_type \
	-DCMAKE_INSTALL_PREFIX=../../.prefix \
	../../uhr/cpp
$make_cmd && $make_cmd install
popd


artifact=unity/Assets/Plugins/x86_64/uhr-windows.x86_64.dll
mkdir -p `dirname $artifact`
cp .prefix/bin/uhr.dll $artifact

# For the benefit of the ci job log:
ls -alR $artifact
dumpbin -NOLOGO -DEPENDENTS $artifact
dumpbin -NOLOGO -EXPORTS $artifact
