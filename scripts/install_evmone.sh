#!/usr/bin/env sh

# This is suitable for CIs, not end users.
# This script is invoked by ossfuzz build CI.
set -e

TEMPDIR="src"
cd /
mkdir -p $TEMPDIR
cd $TEMPDIR
git clone --recurse-submodules https://github.com/chfast/evmone.git
(
  cd evmone
  mkdir build
  cd build
  cmake .. -DBUILD_SHARED_LIBS=OFF
  make -j2
)

git clone https://github.com/chfast/intx.git
(
  cd intx
  mkdir build
  cd build
  cmake .. -DBUILD_SHARED_LIBS=OFF -DINTX_TESTING=OFF -DINTX_BENCHMARKING=OFF
  make -j2
)

git clone https://github.com/chfast/ethash.git
(
  cd ethash
  mkdir build
  cd build
  cmake .. -DBUILD_SHARED_LIBS=OFF -DETHASH_BUILD_TESTS=OFF
  make -j2
)
