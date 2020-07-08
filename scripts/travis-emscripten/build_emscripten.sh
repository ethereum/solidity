#!/usr/bin/env bash

#------------------------------------------------------------------------------
# This script builds the solidity binary using Emscripten.
# Emscripten is a way to compile C/C++ to JavaScript.
#
# http://kripken.github.io/emscripten-site/
#
# First run install_dep.sh OUTSIDE of docker and then
# run this script inside a docker image trzeci/emscripten
#
# The documentation for solidity is hosted at:
#
# http://solidity.readthedocs.io/
#
# ------------------------------------------------------------------------------
# SPDX-License-Identifier: GPL-3.0
#------------------------------------------------------------------------------

set -ev

if test -z "$1"; then
	BUILD_DIR="emscripten_build"
else
	BUILD_DIR="$1"
fi

WORKSPACE=/root/project

echo -en 'travis_fold:start:compiling_solidity\\r'
cd $WORKSPACE
mkdir -p $BUILD_DIR
cd $BUILD_DIR
cmake \
  -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchains/emscripten.cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DBoost_USE_STATIC_LIBS=1 \
  -DBoost_USE_STATIC_RUNTIME=1 \
  -DTESTS=0 \
  ..
make -j 4 soljson
# Patch soljson.js for backwards compatibility.
# TODO: remove this with 0.7.
# "viiiii" encodes the signature of the callback function.
sed -i -e 's/addFunction(func,sig){/addFunction(func,sig){sig=sig||"viiiii";/' libsolc/soljson.js

cd ..
mkdir -p upload
cp $BUILD_DIR/libsolc/soljson.js upload/
cp $BUILD_DIR/libsolc/soljson.js ./

OUTPUT_SIZE=`ls -la soljson.js`

echo "Emscripten output size: $OUTPUT_SIZE"

echo -en 'travis_fold:end:compiling_solidity\\r'
