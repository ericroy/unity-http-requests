#!/bin/bash -e
here="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
pushd $here/..

function verify () {
	local name=$1
	local expected=$2
	local actual=$3
	echo "Verifying $name"
	if [[ "$expected" != "$actual" ]]; then
		echo "SHA1 check failed for: $name"
		echo "  Expected: $expected"
		echo "    Actual: $actual"
		exit 1
	fi
}

curl_version=7.75.0
curl_url=https://curl.se/download/curl-${curl_version}.tar.gz
curl_hash=fbd1e354a5e4e9a4ac07db3d1222d19f84a5e751
curl_file=uhr/cpp/deps/curl-${curl_version}.tar.gz

mbedtls_version=2.25.0
mbedtls_url=https://github.com/ARMmbed/mbedtls/archive/v${mbedtls_version}.tar.gz
mbedtls_hash=746ab10dec0aaadba746c2ecb3df24807329dce1
mbedtls_file=uhr/cpp/deps/mbedtls-${mbedtls_version}.tar.gz


#
# curl
#
if [ -f $curl_file ]; then
	actual_hash=`sha1sum $curl_file`
	verify $curl_file $curl_hash $actual_hash 
else
	echo "Downloading $curl_url"
	actual_hash=`wget -q -O - $curl_url | tee $curl_file | sha1sum`
	verify $curl_file $curl_hash $actual_hash
fi

echo "Extracting curl"
mkdir -p uhr/cpp/deps/curl
tar -xf $curl_file --strip-components=1 -C uhr/cpp/deps/curl


#
# mbedtls
#
if [ -f $mbedtls_file ]; then
	actual_hash=`sha1sum $mbedtls_file`
	verify $mbedtls_file $mbedtls_hash $actual_hash 
else
	echo "Downloading $mbedtls_url"
	actual_hash=`wget -q -O - $mbedtls_url | tee $mbedtls_file | sha1sum`
	verify $mbedtls_file $mbedtls_hash $actual_hash
fi

echo "Extracting mbedtls"
mkdir -p uhr/cpp/deps/mbedtls
tar -xf $mbedtls_file --strip-components=1 -C uhr/cpp/deps/mbedtls


echo "Done"