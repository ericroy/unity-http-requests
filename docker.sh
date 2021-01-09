#!/bin/bash -e
here="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
pushd $here

plat_name=$1
unity_path="$here/unity"
uhr_path="$here/uhr"
if [ "$(expr substr $(uname -s) 1 5)" == "MINGW" ]; then
    # On Windows, docker insists on Windows-style paths
    unity_path="$(cmd.exe /c 'echo %cd%')\unity"
    uhr_path="$(cmd.exe /c 'echo %cd%')\uhr"
fi

docker build -f ./dockerfiles/Dockerfile.$plat_name -t uhr/$plat_name .
docker run --rm -it \
    -v "$uhr_path:/workspace/uhr:ro" \
    -v "$unity_path:/workspace/unity:rw" \
    uhr/$plat_name
