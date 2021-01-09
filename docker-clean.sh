#!/bin/bash -e
here="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
pushd $here

docker rm $(docker ps -a -q) || true
docker image prune -f