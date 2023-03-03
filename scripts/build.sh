#!/usr/bin/env bash
set -e

ROOTDIR="$(dirname "$0")/.."
BUILDDIR="${ROOTDIR}/build"

if [[ $# -eq 0 ]]; then
    BUILD_TYPE=Release
else
    BUILD_TYPE="$1"
fi

if [[ "$(git tag --points-at HEAD 2>/dev/null)" == v* ]]; then
    touch "${ROOTDIR}/prerelease.txt"
fi

mkdir -p "${BUILDDIR}"
cd "${BUILDDIR}"

cmake .. -DCMAKE_BUILD_TYPE="$BUILD_TYPE" "${@:2}"
make -j2

if [[ "$#" -gt 1 && "$2" == "--install" ]]; then
    echo "Build complete. Installing ..."
    if [[ "${CI}" == "" ]]; then
        sudo make install
    else
        make install
    fi
else
    echo "Build complete. Use --install flag to install."
fi
