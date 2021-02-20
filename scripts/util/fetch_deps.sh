#!/bin/bash -e
here="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
pushd $here/../..

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
	if [ -f $file ]; then
		./scripts/util/verify.sh $file $hash || (
			./scripts/util/download.sh $url $file
			./scripts/util/verify.sh $file $hash
		)
	else
		./scripts/util/download.sh $url $file
		./scripts/util/verify.sh $file $hash
	fi
}

get $curl_url $curl_file $curl_hash
get $mbedtls_url $mbedtls_file $mbedtls_hash

echo "Extracting $curl_file"
mkdir -p uhr/cpp/deps/curl
tar -xf $curl_file --strip-components=1 -C uhr/cpp/deps/curl

echo "Extracting $mbedtls_file"
mkdir -p uhr/cpp/deps/mbedtls
tar -xf $mbedtls_file --strip-components=1 -C uhr/cpp/deps/mbedtls
