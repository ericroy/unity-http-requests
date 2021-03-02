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

# mbedtls
mkdir -p .build/mbedtls
pushd .build/mbedtls
cmake -DCMAKE_BUILD_TYPE=$build_type \
    -DCMAKE_INSTALL_PREFIX=../../.prefix \
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=true \
    -DENABLE_TESTING:BOOL=false \
    -DENABLE_PROGRAMS:BOOL=false \
    ../../uhr/cpp/deps/mbedtls
make -j$(nproc)
make install
popd

# lws
mkdir -p .build/lws
pushd .build/lws
cmake -DCMAKE_BUILD_TYPE=$build_type \
	-DCMAKE_INSTALL_PREFIX=../../.prefix \
	-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=true \
	-DLWS_WITH_BUNDLED_ZLIB_DEFAULT:BOOL=true \
	-DLWS_ROLE_WS:BOOL=false \
	-DLWS_WITH_MBEDTLS:BOOL=true \
	-DLWS_WITH_SHARED:BOOL=false \
	-DLWS_WITH_BUNDLED_ZLIB_DEFAULT:BOOL=true \
	-DLWS_WITHOUT_SERVER:BOOL=true \
	-DLWS_WITHOUT_TESTAPPS:BOOL=true \
	-DDISABLE_WERROR:BOOL=true \
    ../../uhr/cpp/deps/lws
make -j$(nproc)
make install
popd

# uhr
mkdir -p .build/uhr
pushd .build/uhr
cmake -DCMAKE_BUILD_TYPE=$build_type \
	-DCMAKE_INSTALL_PREFIX=../../.prefix \
	../../uhr/cpp
make -j$(nproc)
make install
popd

artifact=unity/Assets/Plugins/x86_64/uhr-linux.x86_64.so
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