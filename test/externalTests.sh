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

if [ "$CIRCLECI" ]
then
    function printTask() { echo ""; echo "$(tput bold)$(tput setaf 2)$1$(tput setaf 7)"; }
    function printError() { echo ""; echo "$(tput setaf 1)$1$(tput setaf 7)"; }
    function printLog() { echo "$(tput setaf 3)$1$(tput setaf 7)"; }
else
    function printTask() { echo ""; echo "$(tput bold)$(tput setaf 2)$1$(tput sgr0)"; }
    function printError() { echo ""; echo "$(tput setaf 1)$1$(tput sgr0)"; }
    function printLog() { echo "$(tput setaf 3)$1$(tput sgr0)"; }
fi

if [ ! -f "$1" ]
then
  echo "Usage: $0 <path to soljson.js>"
  exit 1
fi

SOLJSON="$1"
SOLCVERSION="UNDEFINED"

function setup_solcjs
{
  printLog "Setting up solc-js..."
  cd "$1"
  git clone --depth 1 -b v0.5.0 https://github.com/ethereum/solc-js.git solc

  cd solc
  npm install
  cp "$SOLJSON" soljson.js
  SOLCVERSION=$(./solcjs --version)
  cd ..
  echo "Using solcjs version $SOLCVERSION"
}

function download_project
{
  local repo="$1"
  local branch="$2"
  local dir="$3"

  printLog "Cloning $branch of $repo..."
  git clone --depth 1 "$repo" -b "$branch" "$dir/ext"
  cd ext
  echo "Current commit hash: `git rev-parse HEAD`"
}

function setup
{
  local repo="$1"
  local branch="$2"
  local dir="$3"

  setup_solcjs "$dir"
  download_project "$repo" "$branch" "$dir"

  replace_version_pragmas
}

function replace_version_pragmas
{
  # Replace fixed-version pragmas in Gnosis (part of Consensys best practice)
  printLog "Replacing fixed-version pragmas..."
  find contracts test -name '*.sol' -type f -print0 | xargs -0 sed -i -e 's/pragma solidity [\^0-9\.]*/pragma solidity >=0.0/'
}

function replace_libsolc_call
{
  # Change "compileStandard" to "compile" (needed for pre-5.x Truffle)
  printLog "Replacing libsolc compile call in Truffle..."
  sed -i s/solc.compileStandard/solc.compile/ "node_modules/truffle/build/cli.bundled.js"
}

function find_truffle_config
{
  local config_file="truffle.js"
  local alt_config_file="truffle-config.js"

  if [ ! -f "$config_file" ] && [ ! -f "$alt_config_file" ]; then
    printError "No matching Truffle config found."
  fi
  if [ ! -f "$config_file" ]; then
    config_file=alt_config_file
  fi
  echo "$config_file"
}

function force_solc_truffle_modules
{
  # Replace solc package by v0.5.0 and then overwrite with current version.
  printLog "Forcing solc version for all Truffle modules..."
  for d in node_modules node_modules/truffle/node_modules
  do
  (
    if [ -d "$d" ]
    then
      cd $d
      rm -rf solc
      git clone --depth 1 -b v0.5.0 https://github.com/ethereum/solc-js.git solc
      cp "$1" solc/soljson.js
    fi
  )
  done
}

function force_solc
{
  local config_file="$1"
  local dir="$2"

  printLog "Forcing solc version..."
  cat >> "$config_file" <<EOF
module.exports['compilers'] = {solc: {version: "$dir/solc"} };
EOF
}

function force_solc_settings
{
  local config_file="$1"
  local settings="$2"
  local evmVersion="$3"

  printLog "Forcing solc settings..."
  echo "Config file: $config_file"
  echo "Optimizer settings: $settings"
  echo "EVM version: $evmVersion"
  echo ""

  echo "module.exports['compilers']['solc']['settings'] = { optimizer: $settings, evmVersion: \"$evmVersion\" };" >> "$config_file"
}

function verify_compiler_version
{
  local solc_version="$1"

  printLog "Verify that the correct version ($solc_version) of the compiler was used to compile the contracts..."
  grep -e "$solc_version" -r build/contracts > /dev/null
}

function clean
{
  rm -rf build || true
}

# Since Zeppelin 2.1.1 it supports Solidity 0.5.0.
printTask "Testing Zeppelin..."
echo "==========================="
DIR=$(mktemp -d)
(
  setup https://github.com/OpenZeppelin/openzeppelin-solidity.git master "$DIR"

  npm install

  CONFIG="truffle-config.js"

  replace_libsolc_call
  force_solc_truffle_modules "$SOLJSON"
  force_solc "$CONFIG" "$DIR"

  for optimize in "{ enabled: false }" "{ enabled: true }" "{ enabled: true, details: { yul: true } }"
  do
    clean
    force_solc_settings "$CONFIG" "$optimize" "petersburg"

    npx truffle compile
    verify_compiler_version "$SOLCVERSION"
    npm run test
  done
)
rm -rf "$DIR"
echo "Done."

printTask "Testing GnosisSafe..."
echo "==========================="
DIR=$(mktemp -d)
(
  setup https://github.com/gnosis/safe-contracts.git development "$DIR"

  npm install

  CONFIG=$(find_truffle_config)

  force_solc_truffle_modules "$SOLJSON"
  force_solc "$CONFIG" "$DIR"

  for optimize in "{ enabled: false }" "{ enabled: true }" "{ enabled: true, details: { yul: true } }"
  do
    clean
    force_solc_settings "$CONFIG" "$optimize" "petersburg"

    npx truffle compile
    verify_compiler_version "$SOLCVERSION"
    npm test
  done
)
rm -rf "$DIR"
echo "Done."
echo "All external tests passed."

# Disabled temporarily as it needs to be updated to latest Truffle first.
#test_truffle Gnosis https://github.com/axic/pm-contracts.git solidity-050
