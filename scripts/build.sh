#!/usr/bin/env bash
set -e

ROOTDIR="$(dirname "$0")/.."
BUILDDIR="${ROOTDIR}/build"

if [[ $# -eq 0 ]]; then
    BUILD_TYPE=Release
else
    BUILD_TYPE="$1"
fi

mkdir -p "${BUILDDIR}"
cd "${BUILDDIR}"

if [[ "$(git tag --points-at HEAD 2>/dev/null)" == v* ]]; then
	cmake .. -DSOL_FORCE_RELEASE=On -DCMAKE_BUILD_TYPE="$BUILD_TYPE" "${@:2}"
else
	cmake .. -DCMAKE_BUILD_TYPE="$BUILD_TYPE" "${@:2}"
fi

make -j2

if [[ "${CI}" == "" ]]; then
	echo "Installing ..."
	sudo make install
fi
