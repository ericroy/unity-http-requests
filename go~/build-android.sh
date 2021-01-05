#!/bin/bash -e
here="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
pushd $here

CC="$NDK_ROOT/bin/arm-linux-androideabi-gcc" GOOS=linux GOARCH=arm GOARM=7 \
    go build -ldflags="-s -w" -buildmode c-shared -o ../Assets/Plugins/Android/UnityHttpRequests.so ./cmd/unity_http_requests/
rm -f ../Assets/Plugins/Android/UnityHttpRequests.h