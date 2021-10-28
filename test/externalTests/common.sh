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

CURRENT_EVM_VERSION=london

function print_optimizer_levels_or_exit
{
    local selected_levels="$1"

    [[ $selected_levels != "" ]] || { printWarning "No steps to run. Exiting."; exit 0; }

    printLog "Selected optimizer levels: ${selected_levels}"
}

function verify_input
{
    if [ ! -f "$1" ]; then
        printError "Usage: $0 <path to soljson.js>"
        exit 1
    fi
}

function verify_version_input
{
    if [ -z "$1" ] || [ ! -f "$1" ] || [ -z "$2" ]; then
        printError "Usage: $0 <path to soljson.js> <version>"
        exit 1
    fi
}

function setup
{
    local soljson="$1"
    local branch="$2"

    setup_solcjs "$DIR" "$soljson" "$branch" "solc"
    cd solc
}

function setup_solcjs
{
    local dir="$1"
    local soljson="$2"
    local branch="${3:-master}"
    local path="${4:-solc/}"

    cd "$dir"
    printLog "Setting up solc-js..."
    git clone --depth 1 -b "$branch" https://github.com/ethereum/solc-js.git "$path"

    cd "$path"

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
    echo "Current commit hash: $(git rev-parse HEAD)"
}

function force_truffle_version
{
    local version="$1"

    sed -i 's/"truffle":\s*".*"/"truffle": "'"$version"'"/g' package.json
}

function replace_version_pragmas
{
    # Replace fixed-version pragmas (part of Consensys best practice).
    # Include all directories to also cover node dependencies.
    printLog "Replacing fixed-version pragmas..."
    find . test -name '*.sol' -type f -print0 | xargs -0 sed -i -E -e 's/pragma solidity [^;]+;/pragma solidity >=0.0;/'
}

function neutralize_package_lock
{
    # Remove lock files (if they exist) to prevent them from overriding our changes in package.json
    rm --force --verbose yarn.lock
    rm --force --verbose package-lock.json
}

function neutralize_package_json_hooks
{
    [[ -f package.json ]] || fail "package.json not found"
    sed -i 's|"prepublish": *".*"|"prepublish": ""|g' package.json
    sed -i 's|"prepare": *".*"|"prepare": ""|g' package.json
}

function force_solc_modules
{
    local custom_solcjs_path="${1:-solc/}"

    [[ -d node_modules/ ]] || assertFail

    printLog "Replacing all installed solc-js with a link to the latest version..."
    soljson_binaries=$(find node_modules -type f -path "*/solc/soljson.js")
    for soljson_binary in $soljson_binaries
    do
        local solc_module_path
        solc_module_path=$(dirname "$soljson_binary")

        printLog "Found and replaced solc-js in $solc_module_path"
        rm -r "$solc_module_path"
        ln -s "$custom_solcjs_path" "$solc_module_path"
    done
}

function force_truffle_compiler_settings
{
    local config_file="$1"
    local solc_path="$2"
    local level="$3"
    local evm_version="${4:-"$CURRENT_EVM_VERSION"}"

    printLog "Forcing Truffle compiler settings..."
    echo "-------------------------------------"
    echo "Config file: $config_file"
    echo "Compiler path: $solc_path"
    echo "Optimization level: $level"
    echo "Optimizer settings: $(optimizer_settings_for_level "$level")"
    echo "EVM version: $evm_version"
    echo "-------------------------------------"

    # Forcing the settings should always work by just overwriting the solc object. Forcing them by using a
    # dedicated settings objects should only be the fallback.
    echo "module.exports['compilers'] = $(truffle_compiler_settings "$solc_path" "$level" "$evm_version");" >> "$config_file"
}

function truffle_verify_compiler_version
{
    local solc_version="$1"
    local full_solc_version="$2"

    printLog "Verify that the correct version (${solc_version}/${full_solc_version}) of the compiler was used to compile the contracts..."
    grep "$full_solc_version" --with-filename --recursive build/contracts || fail "Wrong compiler version detected."
}

function truffle_clean
{
    rm -rf build/
}

function run_test
{
    local compile_fn="$1"
    local test_fn="$2"

    replace_version_pragmas

    printLog "Running compile function..."
    $compile_fn

    printLog "Running test function..."
    $test_fn
}

function optimizer_settings_for_level
{
    local level="$1"

    case "$level" in
        1) echo "{enabled: false}" ;;
        2) echo "{enabled: true}" ;;
        3) echo "{enabled: true, details: {yul: true}}" ;;
        *)
            printError "Optimizer level not found. Please define OPTIMIZER_LEVEL=[1, 2, 3]"
            exit 1
            ;;
    esac
}

function truffle_compiler_settings
{
    local solc_path="$1"
    local level="$2"
    local evm_version="$3"

    echo "{"
    echo "    solc: {"
    echo "        version: \"${solc_path}\","
    echo "        settings: {"
    echo "            optimizer: $(optimizer_settings_for_level "$level"),"
    echo "            evmVersion: \"${evm_version}\""
    echo "        }"
    echo "    }"
    echo "}"
}

function compile_and_run_test
{
    local compile_fn="$1"
    local test_fn="$2"
    local verify_fn="$3"

    printLog "Running compile function..."
    $compile_fn
    $verify_fn "$SOLCVERSION_SHORT" "$SOLCVERSION"

    if [[ "$COMPILE_ONLY" == 1 ]]; then
        printLog "Skipping test function..."
    else
        printLog "Running test function..."
        $test_fn
    fi
}

function truffle_run_test
{
    local config_file="$1"
    local solc_path="$2"
    local optimizer_level="$3"
    local compile_fn="$4"
    local test_fn="$5"

    truffle_clean
    force_truffle_compiler_settings "$config_file" "$solc_path" "$optimizer_level"
    compile_and_run_test compile_fn test_fn truffle_verify_compiler_version
}

function external_test
{
    local name="$1"
    local main_fn="$2"

    printTask "Testing $name..."
    echo "==========================="
    DIR=$(mktemp -d -t "ext-test-${name}-XXXXXX")
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
