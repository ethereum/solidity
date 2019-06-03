#!/usr/bin/env bash

git clone --depth 1 --branch z3-4.8.1 https://github.com/Z3Prover/z3.git
cd z3
mkdir build
cd build
LDFLAGS="-static" cmake -DBUILD_LIBZ3_SHARED=OFF ..
make -j 4
make install