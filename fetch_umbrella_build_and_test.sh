#!/usr/bin/env bash

set -ev

if [[ "$OSTYPE" != "darwin"* ]]
then
  cd docs && sphinx-build -nW -b html -d _build/doctrees . _build/html && cd ..
fi

SUBREPO=solidity

cd ..
git clone --depth 3 -b develop https://github.com/ethereum/tests.git
export ETHEREUM_TEST_PATH=$(pwd)/tests/
git clone --recursive -b develop https://github.com/ethereum/webthree-umbrella.git
cd webthree-umbrella
rm -rf $SUBREPO
mv ../$SUBREPO .
mkdir build
cd build
OPTIONS=""
if [[ "$OSTYPE" != "darwin"* ]]
then
  OPTIONS="-DCMAKE_C_COMPILER=/usr/lib/ccache/$CC -DCMAKE_CXX_COMPILER=/usr/lib/ccache/$CXX"
fi
cmake .. -DGUI=0 -DCMAKE_BUILD_TYPE=$TRAVIS_BUILD_TYPE $OPTIONS
make lllc solc soljson soltest


./solidity/test/soltest
