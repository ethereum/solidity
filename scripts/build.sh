#!/usr/bin/env bash

if [ -z "$1" ]; then 
    BUILD_TYPE=Release
else 
    BUILD_TYPE="$1"
fi

PWD=$(pwd)
cd $(git rev-parse --show-toplevel)
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
make -j2 && install solc/solc /usr/bin && install test/soltest
cd "$PWD"