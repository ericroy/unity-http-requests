#!/bin/bash -e
here="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
pushd $here

out_dir=../../Assets/Plugins/Editor
artifact_basename=UnityHttpRequests-windows-amd64

mkdir -p $out_dir
GOOS=windows GOARCH=amd64 \
    go build -ldflags="-s -w" -buildmode c-shared -o $out_dir/$artifact_basename.dll ./cmd/unity_http_requests/
rm -f $out_dir/$artifact_basename.h