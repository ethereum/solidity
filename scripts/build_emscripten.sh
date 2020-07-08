#!/usr/bin/env bash

#------------------------------------------------------------------------------
# Bash script for building Solidity for emscripten.
#
# The documentation for solidity is hosted at:
#
#     https://solidity.readthedocs.org
#
# ------------------------------------------------------------------------------
# SPDX-License-Identifier: GPL-3.0
#------------------------------------------------------------------------------

set -e

if test -z "$1"; then
    BUILD_DIR="emscripten_build"
else
    BUILD_DIR="$1"
fi

docker run -v $(pwd):/root/project -w /root/project ethereum/solidity-buildpack-deps:emsdk-1.39.15-2 \
    ./scripts/travis-emscripten/build_emscripten.sh $BUILD_DIR
