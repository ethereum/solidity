#!/usr/bin/env sh

# This script downloads the CMake binary and installs it in ~/.local directory
# (the cmake executable will be in ~/.local/bin).
# This is mostly suitable for CIs, not end users.

set -e

VERSION_MAJOR=3
VERSION_MINOR=15
VERSION_MICRO=2
VERSION=$VERSION_MAJOR.$VERSION_MINOR.$VERSION_MICRO
PREFIX="/usr/local"

OS=$(uname -s)
case $OS in
Linux)  SHA256=f8cbec2abc433938bd9378b129d1d288bb33b8b5a277afe19644683af6e32a59;;
Darwin) SHA256=7ec056d641b8cbea98b220efdcc99da1991758a370063dcac3a0cd388d6b30b6;;
esac

BIN=$PREFIX/bin

PATH=$PREFIX/bin:$PATH

if test -f $BIN/cmake && ($BIN/cmake --version | grep -q "$VERSION"); then
    echo "CMake $VERSION already installed in $BIN"
else
    FILE=cmake-$VERSION-$OS-x86_64.tar.gz
    URL=https://cmake.org/files/v$VERSION_MAJOR.$VERSION_MINOR/$FILE
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
