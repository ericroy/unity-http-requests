#!/bin/bash -e
here="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
pushd $here/../..

jom_url=https://download.qt.io/official_releases/jom/jom_1_1_3.zip
jom_file=.jom/jom.zip
jom_hash=30d7e15d76d3ed04206b6e23f9ce08237eedb77e

mkdir -p .jom
./scripts/util/download.sh $jom_url $jom_file
./scripts/util/verify.sh $jom_file $jom_hash

echo "Extracting jom"
unzip $jom_file jom.exe -d .jom

echo "You must add $PWD/.jom/ to the path!"
