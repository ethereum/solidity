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
      # Replace solc package by v0.5.0
      for d in node_modules node_modules/truffle/node_modules
      do
      (
        cd $d
        rm -rf solc
        git clone --depth 1 -b v0.5.0 https://github.com/ethereum/solc-js.git solc
        cp "$SOLJSON" solc/
      )
      done
      if [ "$name" == "Zeppelin" -o "$name" == "Gnosis" ]; then
        echo "Replaced fixed-version pragmas..."
        # Replace fixed-version pragmas in Gnosis (part of Consensys best practice)
        find contracts test -name '*.sol' -type f -print0 | xargs -0 sed -i -e 's/pragma solidity [\^0-9\.]*/pragma solidity >=0.0/'
      fi
      assertsol="node_modules/truffle/build/Assert.sol"
      if [ -f "$assertsol" ]
      then
        echo "Replace Truffle's Assert.sol with a known good version"
        rm "$assertsol"
        wget https://raw.githubusercontent.com/trufflesuite/truffle-core/ef31bcaa15dbd9bd0f6a0070a5c63f271cde2dbc/lib/testing/Assert.sol -o "$assertsol"
      fi
      # Change "compileStandard" to "compile"
      sed -i s/solc.compileStandard/solc.compile/ "node_modules/truffle/build/cli.bundled.js"
      npx truffle compile
      npm run test
    )
    rm -rf "$DIR"
}

# Since Zeppelin 2.1.1 it supports Solidity 0.5.0.
test_truffle Zeppelin https://github.com/OpenZeppelin/openzeppelin-solidity.git master

# Disabled temporarily as it needs to be updated to latest Truffle first.
#test_truffle Gnosis https://github.com/axic/pm-contracts.git solidity-050

test_truffle GnosisSafe https://github.com/gnosis/safe-contracts.git development
