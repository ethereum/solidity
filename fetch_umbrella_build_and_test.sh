#!/usr/bin/env bash

set -ev

if [[ "$OSTYPE" != "darwin"* ]]
then
  cd docs && sphinx-build -nW -b html -d _build/doctrees . _build/html && cd ..
fi

mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=$TRAVIS_BUILD_TYPE $OPTIONS
make -j4

# Test runs disabled for macos for now,
# we need to find a way to install eth.
if [[ "$OSTYPE" != "darwin"* ]]
then
  git clone --depth 3 -b develop https://github.com/ethereum/tests.git
  export ETHEREUM_TEST_PATH=$(pwd)/tests/

  eth --test -d /tmp/test &
  while [ ! -S /tmp/test/geth.ipc ]; do sleep 2; done
  
  ./solidity/test/soltest --ipc /tmp/test/geth.ipc
  pkill eth
fi
