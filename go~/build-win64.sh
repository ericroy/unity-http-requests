#!/bin/bash -e
here="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
pushd $here

artifact_basename=UnityHttpRequests-windows-amd64

GOOS=windows GOARCH=amd64 \
    go build -ldflags="-s -w" -buildmode c-shared -o ../Assets/Plugins/Editor/$artifact_basename.dll ./cmd/unity_http_requests/
rm -f ../Assets/Plugins/Editor/$artifact_basename.h