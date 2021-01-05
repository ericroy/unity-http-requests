#!/bin/bash -e
here="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
pushd $here

out_dir=../Assets/Plugins/Android
artifact_basename=UnityHttpRequests-linux-arm7

CC_FOR_TARGET="$NDK_ROOT/bin/arm-linux-androideabi-gcc" GOOS=linux GOARCH=arm GOARM=7 \
    go build -ldflags="-s -w" -buildmode c-shared -o $out_dir/$artifact_basename.so ./cmd/unity_http_requests/
rm -f $out_dir/$artifact_basename.h