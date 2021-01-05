#!/bin/bash -e
here="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
pushd $here

GOOS=windows GOARCH=amd64 \
    go build -ldflags="-s -w" -buildmode c-shared -o ../Assets/Plugins/Editor/UnityHttpRequests.dll ./cmd/unity_http_requests/
