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

if [ ! -f "$1" -o -z "$2" ]
then
  echo "Usage: $0 <path to soljson.js> <version>"
  exit 1
fi

SOLJSON="$1"
VERSION="$2"

DIR=$(mktemp -d)
(
    echo "Preparing solc-js (master)..."
    git clone --depth 1 --branch master https://github.com/ethereum/solc-js "$DIR"
    cd "$DIR"
    # disable "prepublish" script which downloads the latest version
    # (we will replace it anyway and it is often incorrectly cached
    # on travis)
    npm config set script.prepublish ''
    npm install

    # Replace soljson with current build
    echo "Replacing soljson.js"
    rm -f soljson.js
    cp "$SOLJSON" soljson.js

    # ensure to use always 0.5.0 sources
    # FIXME: should be removed once the version bump in this repo is done
    rm -rf test/DAO040
    cp -R test/DAO test/DAO040

    # Update version (needed for some tests)
    echo "Updating package.json to version $VERSION"
    npm version --allow-same-version --no-git-tag-version $VERSION

    echo "Running solc-js tests..."
    npm run test
)
rm -rf "$DIR"
