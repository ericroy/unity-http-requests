#!/bin/bash -e
here="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
root="$here/.."
pushd "$root"

if [[ "$(arch)" != "x86_64" ]]; then
    echo "Expected host to be x86_64, but it was $(arch)"
    exit 1
fi

# one of: Debug, Release
build_type="${UHR_BUILD_TYPE:-Debug}"

mkdir -p .build .prefix

# Pull down third party dependencies (curl, mbedtls, etc)
./scripts/util/fetch_deps.sh

common_args=(
    -DCMAKE_FIND_DEBUG_MODE:BOOL=true
    -DCMAKE_BUILD_TYPE="$build_type"
    -DCMAKE_INSTALL_PREFIX="$root/.prefix"
    -DCMAKE_FIND_ROOT_PATH="$root/.prefix"
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=true
)

# utfcpp
mkdir -p .build/utfcpp
pushd .build/utfcpp
cmake "${common_args[@]}" -DUTF8_TESTS:BOOL=false -DUTF8_SAMPLES:BOOL=false -DUTF8_INSTALL:BOOL=true ../../uhr/cpp/deps/utfcpp
make "-j$(nproc)" install
popd

# zlib
mkdir -p .build/zlib
pushd .build/zlib
cmake "${common_args[@]}" -DBUILD_SHARED_LIBS:BOOL=false ../../uhr/cpp/deps/zlib
make "-j$(nproc)" install
popd

# Don't have enough control over curl's cmake build process to
# force it to link zlib statically instead of dynamically.  Work around this
# by just removing the .so so it won't be found.
rm "$root/.prefix/lib/libz.so"

# mbedtls
mkdir -p .build/mbedtls
pushd .build/mbedtls
cmake "${common_args[@]}" -DENABLE_TESTING:BOOL=false -DENABLE_PROGRAMS:BOOL=false ../../uhr/cpp/deps/mbedtls
make "-j$(nproc)" install
popd

# curl
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
    ../../uhr/cpp/deps/curl
make "-j$(nproc)" install
popd

# uhr
mkdir -p .build/uhr
pushd .build/uhr
cmake "${common_args[@]}" ../../uhr/cpp
make "-j$(nproc)" install
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