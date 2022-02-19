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

if jom -version ; then
    generator="NMake Makefiles JOM"
    make_cmd="jom"
else
    generator="NMake Makefiles"
    make_cmd="nmake"
fi

common_args=(
    -G "$generator"
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
$make_cmd
$make_cmd install
popd

# zlib
mkdir -p .build/zlib
pushd .build/zlib
cmake "${common_args[@]}" -DBUILD_SHARED_LIBS:BOOL=false ../../uhr/cpp/deps/zlib
$make_cmd
$make_cmd install
popd

# Don't have enough control over curl's cmake build process to
# force it to link zlib statically instead of dynamically.  Work around this
# by just removing the .dll so it won't be found.
rm -f "$root/.prefix/bin/"zlib{,d}.dll
rm -f "$root/.prefix/lib/"zlib{,d}.lib

# curl
mkdir -p .build/curl
pushd .build/curl
cmake "${common_args[@]}" \
    -DZLIB_ROOT=../../.prefix \
    -DCURL_STATIC_CRT:BOOL=true \
    -DBUILD_SHARED_LIBS:BOOL=false \
    -DBUILD_CURL_EXE:BOOL=false \
    -DBUILD_TESTING:BOOL=false \
    -DHTTP_ONLY:BOOL=true \
    -DCMAKE_USE_LIBSSH2:BOOL=false \
    -DCMAKE_USE_OPENSSL:BOOL=false \
    -DCMAKE_USE_MBEDTLS:BOOL=false \
    -DCMAKE_USE_SCHANNEL:BOOL=true \
    -DCMAKE_USE_ZLIB:BOOL=true \
    ../../uhr/cpp/deps/curl
$make_cmd
$make_cmd install
popd

# uhr
mkdir -p .build/uhr
pushd .build/uhr
cmake "${common_args[@]}" ../../uhr/cpp
$make_cmd
$make_cmd install
popd

artifact=unity/Assets/Plugins/x86_64/uhr-windows.x86_64.dll
mkdir -p "$(dirname "$artifact")"
cp .prefix/bin/uhr.dll "$artifact"

# For the benefit of the ci job log:
ls -alR "$artifact"
dumpbin -NOLOGO -DEPENDENTS "$artifact"
dumpbin -NOLOGO -EXPORTS "$artifact"
