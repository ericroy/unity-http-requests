#!/bin/bash -e
url=$1
dest_file=$2
echo "Downloading $url => $dest_file"
curl -fsSL "$url" > "$dest_file"
