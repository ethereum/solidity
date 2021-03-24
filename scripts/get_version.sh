#!/usr/bin/env bash

#------------------------------------------------------------------------------
# Prints version of the Solidity compiler that the source code corresponds to.
#
# The documentation for solidity is hosted at:
#
#     https://docs.soliditylang.org
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
# (c) 2017 solidity contributors.
#------------------------------------------------------------------------------

set -eu

version=$(sed -n -E -e 's/^\s*set\(PROJECT_VERSION "([0-9.]+)"\)\s*$/\1/p' "$(dirname "$0")/../CMakeLists.txt")

# Sanity check. Sed does not fail if it does not find a match or finds more than one.
[[ $version =~ ^[0-9.]+$ ]] || { echo "Failed to find version in CMakeLists.txt"; exit 1; }

echo "$version"
