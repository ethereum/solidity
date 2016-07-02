#!/usr/bin/env bash

set -ev

if [[ "$OSTYPE" != "darwin"* ]]
then
  cd docs && sphinx-build -nW -b html -d _build/doctrees . _build/html && cd ..
fi

OPTIONS=""
if [[ "$OSTYPE" != "darwin"* ]]
then
  OPTIONS="-DCMAKE_C_COMPILER=/usr/lib/ccache/$CC -DCMAKE_CXX_COMPILER=/usr/lib/ccache/$CXX"
fi

mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=$TRAVIS_BUILD_TYPE $OPTIONS
make -j4

git clone --depth 3 -b develop https://github.com/ethereum/tests.git
export ETHEREUM_TEST_PATH=$(pwd)/tests/

if [[ "$OSTYPE" != "darwin"* ]]
  ./ethbin/eth --test -d /tmp/test &
then
  eth --test -d /tmp/test &
fi

while [ ! -S /tmp/test/geth.ipc ]; do sleep 2; done
  
./test/soltest --ipc /tmp/test/geth.ipc
pkill eth
