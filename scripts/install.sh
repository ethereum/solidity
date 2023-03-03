#!/usr/bin/env bash
set -e

if [[ "$#" -eq 0 ]]; then
    echo "Usage: $0 --from=<build_directory>"
    exit 1
fi

BUILD_DIR=""
for arg in "$@"; do
    case $arg in
        --from=*)
        BUILD_DIR="${arg#*=}"
        ;;
        *)
        echo "Invalid argument: $arg"
        exit 1
        ;;
    esac
done

if [[ ! -d "$BUILD_DIR" ]]; then
    echo "Error: Build directory '$BUILD_DIR' does not exist."
    exit 1
fi

cd "$BUILD_DIR"

if [[ ! -f "Makefile" ]]; then
    echo "Error: Makefile not found in build directory."
    exit 1
fi

if [[ "${CI}" == "" ]]; then
    sudo make install
else
    make install
fi

echo "Installation complete."
