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

# We need git for extracting the commit hash
apt-get update
apt-get -y install git-core

export WORKSPACE=/src

# Boost
echo -en 'travis_fold:start:compiling_boost\\r'
cd "$WORKSPACE"/boost_1_57_0
# if b2 exists, it is a fresh checkout, otherwise it comes from the cache
# and is already compiled
test -e b2 && (
sed -i 's|using gcc ;|using gcc : : /usr/local/bin/em++ ;|g' ./project-config.jam
sed -i 's|$(archiver\[1\])|/usr/local/bin/emar|g' ./tools/build/src/tools/gcc.jam
sed -i 's|$(ranlib\[1\])|/usr/local/bin/emranlib|g' ./tools/build/src/tools/gcc.jam
./b2 link=static variant=release threading=single runtime-link=static \
       thread system regex date_time chrono filesystem unit_test_framework program_options random
find . -name 'libboost*.a' -exec cp {} . \;
rm -rf b2 libs doc tools more bin.v2 status
)
echo -en 'travis_fold:end:compiling_boost\\r'

# Build dependent components and solidity itself
echo -en 'travis_fold:start:compiling_solidity\\r'
cd $WORKSPACE
mkdir -p build
cd build
emcmake cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DEMSCRIPTEN=1 \
  -DBoost_FOUND=1 \
  -DBoost_USE_STATIC_LIBS=1 \
  -DBoost_USE_STATIC_RUNTIME=1 \
  -DBoost_INCLUDE_DIR="$WORKSPACE"/boost_1_57_0/ \
  -DBoost_CHRONO_LIBRARY="$WORKSPACE"/boost_1_57_0/libboost_chrono.a \
  -DBoost_CHRONO_LIBRARIES="$WORKSPACE"/boost_1_57_0/libboost_chrono.a \
  -DBoost_DATE_TIME_LIBRARY="$WORKSPACE"/boost_1_57_0/libboost_date_time.a \
  -DBoost_DATE_TIME_LIBRARIES="$WORKSPACE"/boost_1_57_0/libboost_date_time.a \
  -DBoost_FILESYSTEM_LIBRARY="$WORKSPACE"/boost_1_57_0/libboost_filesystem.a \
  -DBoost_FILESYSTEM_LIBRARIES="$WORKSPACE"/boost_1_57_0/libboost_filesystem.a \
  -DBoost_PROGRAM_OPTIONS_LIBRARY="$WORKSPACE"/boost_1_57_0/libboost_program_options.a \
  -DBoost_PROGRAM_OPTIONS_LIBRARIES="$WORKSPACE"/boost_1_57_0/libboost_program_options.a \
  -DBoost_RANDOM_LIBRARY="$WORKSPACE"/boost_1_57_0/libboost_random.a \
  -DBoost_RANDOM_LIBRARIES="$WORKSPACE"/boost_1_57_0/libboost_random.a \
  -DBoost_REGEX_LIBRARY="$WORKSPACE"/boost_1_57_0/libboost_regex.a \
  -DBoost_REGEX_LIBRARIES="$WORKSPACE"/boost_1_57_0/libboost_regex.a \
  -DBoost_SYSTEM_LIBRARY="$WORKSPACE"/boost_1_57_0/libboost_system.a \
  -DBoost_SYSTEM_LIBRARIES="$WORKSPACE"/boost_1_57_0/libboost_system.a \
  -DBoost_THREAD_LIBRARY="$WORKSPACE"/boost_1_57_0/libboost_thread.a \
  -DBoost_THREAD_LIBRARIES="$WORKSPACE"/boost_1_57_0/libboost_thread.a \
  -DBoost_UNIT_TEST_FRAMEWORK_LIBRARY="$WORKSPACE"/boost_1_57_0/libboost_unit_test_framework.a \
  -DBoost_UNIT_TEST_FRAMEWORK_LIBRARIES="$WORKSPACE"/boost_1_57_0/libboost_unit_test_framework.a \
  -DDev_DEVCORE_LIBRARY="$WORKSPACE"/solidity/build/libdevcore/libsoldevcore.a \
  -DEth_EVMASM_LIBRARY="$WORKSPACE"/solidity/build/libevmasm/libsolevmasm.a \
  -DETH_STATIC=1 -DTESTS=0 \
  ..
emmake make -j 4

cd ..
cp build/solc/soljson.js ./
mkdir -p upload
cp soljson.js upload/

OUTPUT_SIZE=`ls -la build/solc/soljson.js`

echo "Emscripten output size: ${OUTPUT_SIZE}"

echo -en 'travis_fold:end:compiling_solidity\\r'
