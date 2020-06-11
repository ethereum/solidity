#! /bin/bash
#------------------------------------------------------------------------------
# Bash script that install hera ewasm evmc vm. Script will be removed once
# hera is part of the docker images.
#
# The documentation for solidity is hosted at:
#
#     https://solidity.readthedocs.org
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
# (c) 2016-2019 solidity contributors.
# ------------------------------------------------------------------------------

apt-get update
apt-get upgrade -y

cd /usr/src || exit
git clone --branch="v0.3.0" --recurse-submodules https://github.com/ewasm/hera.git
cd hera || exit
mkdir build
cd build || exit
cmake -G Ninja -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX="/usr" ..
ninja
ninja install/strip
rm -rf /usr/src/hera
