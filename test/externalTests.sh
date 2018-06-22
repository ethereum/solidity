#!/usr/bin/env bash

#------------------------------------------------------------------------------
# Bash script to run external Solidity tests.
#
# Argument: Path to soljson.js to test.
#
# Requires npm, networking access and git to download the tests.
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

set -e

if [ ! -f "$1" ]
then
  echo "Usage: $0 <path to soljson.js>"
  exit 1
fi

SOLJSON="$1"

function test_truffle
{
    name="$1"
    repo="$2"
    branch="$3"
    echo "Running $name tests..."
    DIR=$(mktemp -d)
    (
      if [ -n "$branch" ]
      then
        echo "Cloning $branch of $repo..."
        git clone --depth 1 "$repo" -b "$branch" "$DIR"
      else
        echo "Cloning $repo..."
        git clone --depth 1 "$repo" "$DIR"
      fi
      cd "$DIR"
      echo "Current commit hash: `git rev-parse HEAD`"
      npm install
      find . -name soljson.js -exec cp "$SOLJSON" {} \;
      if [ "$name" == "Gnosis" ]; then
        echo "Replaced fixed-version pragmas..."
        # Replace fixed-version pragmas in Gnosis (part of Consensys best practice)
        find contracts test -name '*.sol' -type f -print0 | xargs -0 sed -i -e 's/pragma solidity 0/pragma solidity ^0/'
      fi
      npm run test
    )
    rm -rf "$DIR"
}

# Using our temporary fork here. Hopefully to be merged into upstream after the 0.5.0 release.
test_truffle Zeppelin https://github.com/axic/openzeppelin-solidity.git solidity-050

# Disabled temporarily as it needs to be updated to latest Truffle first.
#test_truffle Gnosis https://github.com/axic/pm-contracts.git solidity-050
