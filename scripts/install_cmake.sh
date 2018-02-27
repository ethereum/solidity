#!/usr/bin/env sh

# This script downloads the CMake binary and installs it in ~/.local directory
# (the cmake executable will be in ~/.local/bin).
# This is mostly suitable for CIs, not end users.

set -e

VERSION=3.7.1
PREFIX=~/.local

OS=$(uname -s)
case $OS in
Linux)  SHA256=7b4b7a1d9f314f45722899c0521c261e4bfab4a6b532609e37fef391da6bade2;;
Darwin) SHA256=1851d1448964893fdc5a8c05863326119f397a3790e0c84c40b83499c7960267;;
esac


BIN=$PREFIX/bin

PATH=$PREFIX/bin:$PATH

if test -f $BIN/cmake && ($BIN/cmake --version | grep -q "$VERSION"); then
    echo "CMake $VERSION already installed in $BIN"
else
    FILE=cmake-$VERSION-$OS-x86_64.tar.gz
    URL=https://cmake.org/files/v3.7/$FILE
    ERROR=0
    TMPFILE=$(mktemp --tmpdir cmake-$VERSION-$OS-x86_64.XXXXXXXX.tar.gz)
    echo "Downloading CMake ($URL)..."
    wget "$URL" -O "$TMPFILE" -nv
    if ! (shasum -a256 "$TMPFILE" | grep -q "$SHA256"); then
        echo "Checksum mismatch ($TMPFILE)"
        exit 1
    fi
    mkdir -p "$PREFIX"
    tar xzf "$TMPFILE" -C "$PREFIX" --strip 1
    rm $TMPFILE
fi
