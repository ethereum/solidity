#!/usr/bin/env bash

#------------------------------------------------------------------------------
# Bash script to execute the Solidity tests.
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
# (c) 2017 solidity contributors.
#------------------------------------------------------------------------------

set -e

REPO_ROOT="$(dirname "$0")"/..

cd $REPO_ROOT/build

echo "Preparing solc-js..."
rm -rf solc-js
git clone https://github.com/ethereum/solc-js
cd solc-js
npm install

# Replace soljson with current build
echo "Replacing soljson.js"
rm -f soljson.js
# Make a copy because paths might not be absolute
cp ../solc/soljson.js soljson.js

# Update version (needed for some tests)
echo "Creating version.c"
rm -f version version.c
cat <<EOF > version.c
#include <stdio.h>
#include "../include/solidity/BuildInfo.h"
int main(int argc, char **argv) {
  printf(ETH_PROJECT_VERSION);
  return 0;
}
EOF
echo "Compiling version.c"
gcc -o version version.c
echo "Running version"
VERSION=`./version`
echo "Updating package.json to version $VERSION"
npm version $VERSION

echo "Running solc-js tests..."
npm run test
