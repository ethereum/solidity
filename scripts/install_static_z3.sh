#!/usr/bin/env bash

set -eu

git clone --depth 1 --branch z3-4.8.7 https://github.com/Z3Prover/z3.git
cd z3
mkdir build
cd build
LDFLAGS="-static" cmake -DZ3_BUILD_LIBZ3_SHARED=OFF ..
make -j 4
make install