#!/bin/bash -e
here="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
pushd $here/../..

lws_version=4.1.6
lws_url=https://github.com/warmcat/libwebsockets/archive/v${lws_version}.tar.gz
lws_hash=c1388f2411ce2e7c57243f8ac1cc52240145ee91
lws_file=uhr/cpp/deps/lws-${lws_version}.tar.gz

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

get $lws_url $lws_file $lws_hash
get $mbedtls_url $mbedtls_file $mbedtls_hash

echo "Extracting $lws_file"
mkdir -p uhr/cpp/deps/lws
tar -xf $lws_file --strip-components=1 -C uhr/cpp/deps/lws

echo "Extracting $mbedtls_file"
mkdir -p uhr/cpp/deps/mbedtls
tar -xf $mbedtls_file --strip-components=1 -C uhr/cpp/deps/mbedtls
