#!/usr/bin/env bash

#------------------------------------------------------------------------------
# Bash script for installing pre-requisite packages for building solidity
# using Emscripten on Ubuntu Trusty.
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

echo -en 'travis_fold:start:installing_dependencies\\r'
test -e boost_1_68_0 -a -e boost_1_68_0/boost || (
rm -rf boost_1_68_0
rm -f boost.tar.xz
wget -q 'https://sourceforge.net/projects/boost/files/boost/1.68.0/boost_1_68_0.tar.gz/download'\
 -O boost.tar.xz
test "$(shasum boost.tar.xz)" = "a78cf6ebb111a48385dd0c135e145a6819a8c856  boost.tar.xz"
tar -xzf boost.tar.xz
rm boost.tar.xz
cd boost_1_68_0
./bootstrap.sh
wget -q 'https://raw.githubusercontent.com/tee3/boost-build-emscripten/master/emscripten.jam'
test "$(shasum emscripten.jam)" = "a7e13fc2c1e53b0e079ef440622f879aa6da3049  emscripten.jam"
echo "using emscripten : : em++ ;" >> project-config.jam
)
cd ..
echo -en 'travis_fold:end:installing_dependencies\\r'
