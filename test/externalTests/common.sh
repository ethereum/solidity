#!/usr/bin/env bash

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
# (c) 2019 solidity contributors.
#------------------------------------------------------------------------------
set -e

# Requires "${REPO_ROOT}/scripts/common.sh" to be included before.

function verify_input
{
    if [ ! -f "$1" ]; then
        printError "Usage: $0 <path to soljson.js>"
        exit 1
    fi
}

function setup_solcjs
{
    local dir="$1"
    local soljson="$2"

    cd "$dir"
    printLog "Setting up solc-js..."
    git clone --depth 1 -b v0.5.0 https://github.com/ethereum/solc-js.git solc

    cd solc
    npm install
    cp "$soljson" soljson.js
    SOLCVERSION=$(./solcjs --version)
    printLog "Using solcjs version $SOLCVERSION"
    cd ..
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

    setup_solcjs "$DIR" "$SOLJSON"
    download_project "$repo" "$branch" "$DIR"
}

function replace_version_pragmas
{
    # Replace fixed-version pragmas (part of Consensys best practice).
    # Include all directories to also cover node dependencies.
    printLog "Replacing fixed-version pragmas..."
    find . test -name '*.sol' -type f -print0 | xargs -0 sed -i -e 's/pragma solidity [\^0-9\.]*/pragma solidity >=0.0/'
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
        if [ -d "$d" ]; then
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
    local soljson="$3"

    force_solc_truffle_modules "$soljson"

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
    echo "-------------------------------------"
    echo "Config file: $config_file"
    echo "Optimizer settings: $settings"
    echo "EVM version: $evmVersion"
    echo "-------------------------------------"

    # Forcing the settings should always work by just overwriting the solc object. Forcing them by using a
    # dedicated settings objects should only be the fallback.
    echo "module.exports['solc'] = { optimizer: $settings, evmVersion: \"$evmVersion\" };" >> "$config_file"
    echo "module.exports['compilers']['solc']['settings'] = { optimizer: $settings, evmVersion: \"$evmVersion\" };" >> "$config_file"
}

function force_abi_v2
{
    # Add "pragma experimental ABIEncoderV2" to all files.
    printLog "Forcibly enabling ABIEncoderV2..."
    find contracts test -name '*.sol' -type f -print0 | \
    while IFS= read -r -d '' file
    do
        # Only add the pragma if it is not already there.
        if grep -q -v 'pragma experimental ABIEncoderV2' "$file"; then
            sed -i -e '1 i pragma experimental ABIEncoderV2;' "$file"
        fi
    done
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

function run_install
{
    local init_fn="$1"
    printLog "Running install function..."
    $init_fn
}

function run_test
{
    local compile_fn="$1"
    local test_fn="$2"

    replace_version_pragmas
    force_solc "$CONFIG" "$DIR" "$SOLJSON"

    printLog "Checking optimizer level..."
    if [ -z "$OPTIMIZER_LEVEL" ]; then
        printError "Optimizer level not found. Please define OPTIMIZER_LEVEL=[1, 2, 3]"
        exit 1
    fi
    if [[ "$OPTIMIZER_LEVEL" == 1 ]]; then
        declare -a optimizer_settings=("{ enabled: false }" "{ enabled: true }" "{ enabled: true, details: { yul: true } }")
    fi
    if [[ "$OPTIMIZER_LEVEL" == 2 ]]; then
        declare -a optimizer_settings=("{ enabled: true }" "{ enabled: true, details: { yul: true } }")
    fi
    if [[ "$OPTIMIZER_LEVEL" == 3 ]]; then
        declare -a optimizer_settings=("{ enabled: true, details: { yul: true } }")
    fi

    for optimize in "${optimizer_settings[@]}"
    do
        clean
        force_solc_settings "$CONFIG" "$optimize" "istanbul"
        # Force ABIEncoderV2 in the last step. Has to be the last because code is modified.
        if [ "$FORCE_ABIv2" = true ]; then
            [[ "$optimize" =~ yul ]] && force_abi_v2
        fi

        printLog "Running compile function..."
        $compile_fn
        verify_compiler_version "$SOLCVERSION"
        printLog "Running test function..."
        $test_fn
    done
}

function external_test
{
    local name="$1"
    local main_fn="$2"

    printTask "Testing $name..."
    echo "==========================="
    DIR=$(mktemp -d)
    (
        if [ -z "$main_fn" ]; then
            printError "Test main function not defined."
            exit 1
        fi
        $main_fn
    )
    rm -rf "$DIR"
    echo "Done."
}

