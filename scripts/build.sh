#!/usr/bin/env bash
set -e

# Check if git is installed
if ! command -v git &> /dev/null; then
    echo "Error: git is not installed. Please install git and try again."
    exit 1
fi

ROOTDIR="$(dirname "$0")/.."
BUILDDIR="${ROOTDIR}/build"

if [[ $# -eq 0 ]]; then
    BUILD_TYPE=Release
else
    BUILD_TYPE="$1"
fi

# Check the first git tag that points to the current commit and starts with "v"
if [[ "$(git tag --points-at HEAD 2>/dev/null | head -n 1)" == v* ]]; then
	touch "${ROOTDIR}/prerelease.txt"
fi

mkdir -p "${BUILDDIR}"
cd "${BUILDDIR}"

cmake .. -DCMAKE_BUILD_TYPE="$BUILD_TYPE" "${@:2}"
make -j2

if [[ "${CI}" == "" ]]; then
	echo "Installing ..."
	sudo make install
fi
