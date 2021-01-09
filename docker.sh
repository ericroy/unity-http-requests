#!/bin/bash -e
here="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
pushd $here

plat_name=$1
assets_path="$here/unity/Assets"

# On Windows, docker insists on Windows-style paths
if [ "$(expr substr $(uname -s) 1 5)" == "MINGW" ]; then
    assets_path="$(cmd.exe /c 'echo %cd%')\unity\Assets"
fi

echo "Using assets path: $assets_path"

# Build the docker image
docker build -f ./dockerfiles/Dockerfile.$plat_name -t uhr/$plat_name .

# For everything except the tests, mount the Assets directory in the workspace
volumes=""
if [ "$plat_name" == "test" ]; then
    volumes="-v '$assets_path:/workspace/Assets:rw'"
fi

# Run the docker image
docker run --rm -it $volumes uhr/$plat_name
