#!/bin/bash
set -euo pipefail
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

# mbedtls
mkdir -p .build/mbedtls
pushd .build/mbedtls
cmake -G "$generator" \
	-DCMAKE_BUILD_TYPE=$build_type \
    -DCMAKE_INSTALL_PREFIX=../../.prefix \
    -DENABLE_TESTING:BOOL=false \
    -DENABLE_PROGRAMS:BOOL=false \
    ../../uhr/cpp/deps/mbedtls
$make_cmd
$make_cmd install
popd

# lws
mkdir -p .build/lws
pushd .build/lws
cmake -G "$generator" \
    -DCMAKE_BUILD_TYPE=$build_type \
	-DCMAKE_INSTALL_PREFIX=../../.prefix \
	-DLWS_WITH_BUNDLED_ZLIB_DEFAULT:BOOL=true \
	-DLWS_ROLE_WS:BOOL=false \
	-DLWS_WITH_MBEDTLS:BOOL=true \
	-DLWS_WITH_SHARED:BOOL=false \
	-DLWS_WITH_BUNDLED_ZLIB_DEFAULT:BOOL=true \
	-DLWS_WITHOUT_SERVER:BOOL=true \
	-DLWS_WITHOUT_TESTAPPS:BOOL=true \
	-DDISABLE_WERROR:BOOL=true \
    ../../uhr/cpp/deps/lws
$make_cmd
$make_cmd install
popd

# uhr
mkdir -p .build/uhr
pushd .build/uhr
cmake -G "$generator" \
    -DCMAKE_BUILD_TYPE=$build_type \
	-DCMAKE_INSTALL_PREFIX=../../.prefix \
	../../uhr/cpp
$make_cmd
$make_cmd install
popd


artifact=unity/Assets/Plugins/x86_64/uhr-windows.x86_64.dll
mkdir -p `dirname $artifact`
cp .prefix/bin/uhr.dll $artifact

# For the benefit of the ci job log:
ls -alR $artifact
dumpbin -NOLOGO -DEPENDENTS $artifact
dumpbin -NOLOGO -EXPORTS $artifact
