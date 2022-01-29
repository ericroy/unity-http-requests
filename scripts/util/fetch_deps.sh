#!/bin/bash -e
here="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
pushd "$here/../.."

utfcpp_version=3.2.1
utfcpp_url=https://github.com/nemtrif/utfcpp/archive/refs/tags/v${utfcpp_version}.tar.gz
utfcpp_hash=04c614fc761a03de239fd290760800dc26091046
utfcpp_file=uhr/cpp/deps/utfcpp-${utfcpp_version}.tar.gz

zlib_version=1.2.11
zlib_url=http://zlib.net/fossils/zlib-${zlib_version}.tar.gz
zlib_hash=e6d119755acdf9104d7ba236b1242696940ed6dd
zlib_file=uhr/cpp/deps/zlib-${zlib_version}.tar.gz

curl_version=7.75.0
curl_url=https://curl.se/download/curl-${curl_version}.tar.gz
curl_hash=fbd1e354a5e4e9a4ac07db3d1222d19f84a5e751
curl_file=uhr/cpp/deps/curl-${curl_version}.tar.gz

mbedtls_version=2.25.0
mbedtls_url=https://github.com/ARMmbed/mbedtls/archive/v${mbedtls_version}.tar.gz
mbedtls_hash=746ab10dec0aaadba746c2ecb3df24807329dce1
mbedtls_file=uhr/cpp/deps/mbedtls-${mbedtls_version}.tar.gz

function get () {
	local url=$1
	local file=$2
	local hash=$3
	if [ -f "$file" ]; then
		./scripts/util/verify.sh "$file" "$hash" || (
			./scripts/util/download.sh "$url" "$file"
			./scripts/util/verify.sh "$file" "$hash"
		)
	else
		./scripts/util/download.sh "$url" "$file"
		./scripts/util/verify.sh "$file" "$hash"
	fi
}

get $utfcpp_url $utfcpp_file $utfcpp_hash
get $zlib_url $zlib_file $zlib_hash
get $curl_url $curl_file $curl_hash
get $mbedtls_url $mbedtls_file $mbedtls_hash

echo "Extracting $utfcpp_file"
mkdir -p uhr/cpp/deps/utfcpp
tar -xf $utfcpp_file --strip-components=1 -C uhr/cpp/deps/utfcpp

echo "Extracting $zlib_file"
mkdir -p uhr/cpp/deps/zlib
tar -xf $zlib_file --strip-components=1 -C uhr/cpp/deps/zlib

echo "Extracting $curl_file"
mkdir -p uhr/cpp/deps/curl
tar -xf $curl_file --strip-components=1 -C uhr/cpp/deps/curl

echo "Extracting $mbedtls_file"
mkdir -p uhr/cpp/deps/mbedtls
tar -xf $mbedtls_file --strip-components=1 -C uhr/cpp/deps/mbedtls
