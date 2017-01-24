#!/usr/bin/env bash

if [ -z "$1" ]; then 
    BUILD_TYPE=Release
else 
    BUILD_TYPE="$1"
fi

mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
make -j2
cd ..