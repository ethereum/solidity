#!/usr/bin/env bash

#------------------------------------------------------------------------------
# This script builds the solidity binary using Emscripten.
# Emscripten is a way to compile C/C++ to JavaScript.
#
# http://kripken.github.io/emscripten-site/
#
# First run install_dep.sh OUTSIDE of docker and then
# run this script inside a docker image trzeci/emscripten
#
# The documentation for solidity is hosted at:
#
# https://docs.soliditylang.org
#
# ------------------------------------------------------------------------------
# This file is part of solidity.
#
# solidity is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# solidity is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with solidity.  If not, see <http://www.gnu.org/licenses/>
#
# (c) 2016 solidity contributors.
#------------------------------------------------------------------------------

set -ev

SCRIPT_DIR="$(realpath "$(dirname "$0")/..")"
# shellcheck source=scripts/common.sh
source "${SCRIPT_DIR}/common.sh"
ROOT_DIR="${SCRIPT_DIR}/.."

function build() {
    local build_dir="$1"
    local prerelease_source="${2:-ci}"

    cd "${ROOT_DIR}"

    "${SCRIPT_DIR}/prerelease_suffix.sh" "$prerelease_source" "$(git tag --points-at HEAD 2> /dev/null)" > prerelease.txt

    export CCACHE_DIR="${CCACHE_DIR:-$HOME/.ccache}"
    CCACHE_BASEDIR="$(pwd)"
    export CCACHE_BASEDIR
    export CCACHE_NOHASHDIR=1
    mkdir -p "$CCACHE_DIR"
    ccache -z

    mkdir -p "$build_dir"
    cd "$build_dir"
    emcmake cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DBoost_USE_STATIC_LIBS=1 \
        -DBoost_USE_STATIC_RUNTIME=1 \
        -DTESTS=0 \
    ..
    make soljson
    ccache -s

    cd ..
    mkdir -p upload
    scripts/ci/pack_soljson.sh "$build_dir/libsolc/soljson.js" "$build_dir/libsolc/soljson.wasm" upload/soljson.js
    cp upload/soljson.js ./

    OUTPUT_SIZE=$(ls -la soljson.js)

    echo "Emscripten output size: $OUTPUT_SIZE"
}

function show_help() {
cat << EOF
Usage: ${0##*/} [-h|--help] [--build-dir DIR] [--prerelease-source prerelease_source]
Build Solidity emscripten binary
    -h | --help          Display this help message
    --build-dir          The emscripten build directory
    --prerelease-source  The prerelease source string. E.g. 'nightly' or 'ci'.
EOF
}

function main() {
    local build_dir="emscripten_build"
    local prerelease_source=""

    while (( $# > 0 )); do
        case "$1" in
            -h|--help)
                show_help
                exit 0
                ;;
            --build-dir)
                [[ -n "$2" ]] || fail "Option --build-dir cannot be empty"
                build_dir="$2"
                shift 2
                ;;
            --prerelease-source)
                [[ -n "$2" ]] || fail "Option --prerelease-source cannot be empty"
                prerelease_source="$2"
                shift 2
                ;;
            *) fail "Invalid option: $1" ;;
        esac
    done
    build "$build_dir" "$prerelease_source"
}

main "$@"
