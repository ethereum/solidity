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
# This file is part of solidity.
#
# solidity is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# solidity is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with solidity.  If not, see <http://www.gnu.org/licenses/>
#
# (c) 2016 solidity contributors.
#------------------------------------------------------------------------------

set -ev

if ! type git &>/dev/null; then
    # We need git for extracting the commit hash
    apt-get update
    apt-get -y install git-core
fi

if ! type wget &>/dev/null; then
    # We need wget to install cmake
    apt-get update
    apt-get -y install wget
fi

WORKSPACE=/root/project

# Increase nodejs stack size
if ! [ -e /emsdk_portable/node/bin/node_orig ]
then
  mv /emsdk_portable/node/bin/node /emsdk_portable/node/bin/node_orig
  echo -e '#!/bin/sh\nexec /emsdk_portable/node/bin/node_orig --stack-size=8192 $@' > /emsdk_portable/node/bin/node
  chmod 755 /emsdk_portable/node/bin/node
fi

# Boost
echo -en 'travis_fold:start:compiling_boost\\r'
cd "$WORKSPACE"/boost_1_68_0
# if b2 exists, it is a fresh checkout, otherwise it comes from the cache
# and is already compiled
test -e b2 && (
./b2 toolset=emscripten link=static variant=release threading=single runtime-link=static \
       system regex filesystem unit_test_framework program_options cxxflags="-Wno-unused-local-typedef -Wno-variadic-macros -Wno-c99-extensions -Wno-all"
find . -name 'libboost*.a' -exec cp {} . \;
rm -rf b2 libs doc tools more bin.v2 status
)
echo -en 'travis_fold:end:compiling_boost\\r'

echo -en 'travis_fold:start:install_cmake.sh\\r'
source $WORKSPACE/scripts/install_cmake.sh
echo -en 'travis_fold:end:install_cmake.sh\\r'

# Build dependent components and solidity itself
echo -en 'travis_fold:start:compiling_solidity\\r'
cd $WORKSPACE
mkdir -p build
cd build
cmake \
  -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchains/emscripten.cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DBoost_FOUND=1 \
  -DBoost_USE_STATIC_LIBS=1 \
  -DBoost_USE_STATIC_RUNTIME=1 \
  -DBoost_INCLUDE_DIR="$WORKSPACE"/boost_1_68_0/ \
  -DBoost_FILESYSTEM_LIBRARY_RELEASE="$WORKSPACE"/boost_1_68_0/libboost_filesystem.a \
  -DBoost_PROGRAM_OPTIONS_LIBRARY_RELEASE="$WORKSPACE"/boost_1_68_0/libboost_program_options.a \
  -DBoost_REGEX_LIBRARY_RELEASE="$WORKSPACE"/boost_1_68_0/libboost_regex.a \
  -DBoost_SYSTEM_LIBRARY_RELEASE="$WORKSPACE"/boost_1_68_0/libboost_system.a \
  -DBoost_UNIT_TEST_FRAMEWORK_LIBRARY_RELEASE="$WORKSPACE"/boost_1_68_0/libboost_unit_test_framework.a \
  -DTESTS=0 \
  ..
make -j 4

cd ..
mkdir -p upload
# Patch soljson.js to provide backwards-compatibility with older emscripten versions
echo ";/* backwards compatibility */ Module['Runtime'] = Module;" >> build/libsolc/soljson.js
cp build/libsolc/soljson.js upload/
cp build/libsolc/soljson.js ./

OUTPUT_SIZE=`ls -la soljson.js`

echo "Emscripten output size: $OUTPUT_SIZE"

echo -en 'travis_fold:end:compiling_solidity\\r'
