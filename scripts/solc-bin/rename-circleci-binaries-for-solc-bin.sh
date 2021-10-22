#!/usr/bin/env bash

set -euo pipefail

if (( $# > 0 )) && [[ $1 == --help ]]; then
    echo "Renames binaries following the naming convention from Solidity release page on Github to match"
    echo "the naming convention used in solc-bin."
    echo "Assumes that the binaries are present in the current working directory."
    echo "Obtains version from the --version output of the Linux binary."
    echo
    echo "WARNING: The binaries will be overwritten if they already exist."
    echo
    echo
    echo "Usage:"
    echo "    ./$(basename "$0") --help"
    echo "    ./$(basename "$0") [solc_bin_dir]"
    echo
    echo "    solc_bin_dir     Location of the solc-bin directory."
    echo "                     Default: current working directory."
    echo
    echo
    echo "Examples:"
    echo "    ./$(basename "$0") --help"
    echo "    ./$(basename "$0") ~/solc-bin/"

    exit 0
fi

(( $# <= 1 )) || { >&2 printf "ERROR: Too many arguments"; exit 1; }

solc_bin_dir="${1:-$PWD}"

full_version=$(
    ./solc-static-linux --version |
    sed -En 's/^Version: ([0-9.]+.*\+commit\.[0-9a-f]+(\.mod)?).*$/\1/p'
)

target=linux-amd64
mkdir -p "${solc_bin_dir}/${target}"
mv solc-static-linux "${solc_bin_dir}/${target}/solc-${target}-${full_version}"

target=macosx-amd64
mkdir -p "${solc_bin_dir}/${target}"
mv solc-macos "${solc_bin_dir}/${target}/solc-${target}-${full_version}"

target=windows-amd64
mkdir -p "${solc_bin_dir}/${target}"
mv solc-windows.exe "${solc_bin_dir}/${target}/solc-${target}-${full_version}.exe"

target=bin
mkdir -p "${solc_bin_dir}/${target}"
mv soljson.js "${solc_bin_dir}/${target}/soljson-${full_version}.js"
