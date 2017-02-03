#!/usr/bin/env bash

if [ -z "$1" ]; then 
    BUILD_TYPE=Release
else 
    BUILD_TYPE="$1"
fi

cd $(dirname "$0")
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
make -j2

if [ -z $CI ]; then
	install solc/solc /usr/local/bin && install test/soltest /usr/local/bin
fi