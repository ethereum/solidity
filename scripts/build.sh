#!/usr/bin/env bash

if [ -z "$1" ]; then
    BUILD_TYPE=Release
else
    BUILD_TYPE="$1"
fi

cd $(dirname "$0")/.. &&

if [[ "$(git tag --points-at HEAD 2>/dev/null)" == v* ]]; then
	touch prerelease.txt
fi

mkdir -p build &&
cd build &&
cmake .. -DCMAKE_BUILD_TYPE="$BUILD_TYPE" &&
make -j2

if [ $? -ne 0 ]; then
	echo "Failed to build"
	exit 1
fi

if [ -z $CI ]; then
	echo "Installing solc and soltest"
	install solc/solc /usr/local/bin && install test/soltest /usr/local/bin
fi
