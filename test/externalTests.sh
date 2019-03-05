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
      cd "$DIR"
      git clone --depth 1 -b v0.5.0 https://github.com/ethereum/solc-js.git solc
      SOLCVERSION="UNDEFINED"

      cd solc
      npm install
      cp "$SOLJSON" soljson.js
      SOLCVERSION=$(./solcjs --version)
      cd ..
      echo "Using solcjs version $SOLCVERSION"

      if [ -n "$branch" ]
      then
        echo "Cloning $branch of $repo..."
        git clone --depth 1 "$repo" -b "$branch" "$DIR/ext"
      else
        echo "Cloning $repo..."
        git clone --depth 1 "$repo" "$DIR/ext"
      fi
      cd ext
      echo "Current commit hash: `git rev-parse HEAD`"
      npm ci
      # Replace solc package by v0.5.0
      for d in node_modules node_modules/truffle/node_modules
      do
      (
        if [ -d "$d" ]
        then
          cd $d
          rm -rf solc
          git clone --depth 1 -b v0.5.0 https://github.com/ethereum/solc-js.git solc
          cp "$SOLJSON" solc/soljson.js
        fi
      )
      done
      if [ "$name" == "Zeppelin" -o "$name" == "Gnosis" ]; then
        echo "Replaced fixed-version pragmas..."
        # Replace fixed-version pragmas in Gnosis (part of Consensys best practice)
        find contracts test -name '*.sol' -type f -print0 | xargs -0 sed -i -e 's/pragma solidity [\^0-9\.]*/pragma solidity >=0.0/'
      fi
      # Change "compileStandard" to "compile" (needed for pre-5.x Truffle)
      sed -i s/solc.compileStandard/solc.compile/ "node_modules/truffle/build/cli.bundled.js"
      # Force usage of correct solidity binary (only works with Truffle 5.x)
      cat >> truffle*.js <<EOF
module.exports['compilers'] = {solc: {version: "$DIR/solc"} };
EOF

      npx truffle compile
      echo "Verify that the correct version ($SOLCVERSION) of the compiler was used to compile the contracts..."
      grep -e "$SOLCVERSION" -r build/contracts > /dev/null
      npm run test
    )
    rm -rf "$DIR"
}

# Since Zeppelin 2.1.1 it supports Solidity 0.5.0.
test_truffle Zeppelin https://github.com/OpenZeppelin/openzeppelin-solidity.git master

# Disabled temporarily as it needs to be updated to latest Truffle first.
#test_truffle Gnosis https://github.com/axic/pm-contracts.git solidity-050

# Disabled temporarily because it is incompatible with petersburg EVM and
# there is no easy way to set the EVM version in truffle pre 5.0.
#test_truffle GnosisSafe https://github.com/gnosis/safe-contracts.git development
