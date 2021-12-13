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
    local binary_type="$1"
    local binary_path="$2"

    (( $# == 2 )) || fail "Usage: $0 native|solcjs <path to solc or soljson.js>"
    [[ $binary_type == native || $binary_type == solcjs ]] || fail "Invalid binary type: '${binary_type}'. Must be either 'native' or 'solcjs'."
    [[ -f "$binary_path" ]] || fail "The compiler binary does not exist at '${binary_path}'"
}

function setup_solc
{
    local test_dir="$1"
    local binary_type="$2"
    local binary_path="$3"
    local solcjs_branch="${4:-master}"
    local install_dir="${5:-solc/}"

    [[ $binary_type == native || $binary_type == solcjs ]] || assertFail

    cd "$test_dir"

    if [[ $binary_type == solcjs ]]
    then
        printLog "Setting up solc-js..."
        git clone --depth 1 -b "$solcjs_branch" https://github.com/ethereum/solc-js.git "$install_dir"

        pushd "$install_dir"
        npm install
        cp "$binary_path" soljson.js
        SOLCVERSION=$(./solcjs --version)
        popd
    else
        printLog "Setting up solc..."
        SOLCVERSION=$("$binary_path" --version | tail -n 1 | sed -n -E 's/^Version: (.*)$/\1/p')
    fi

    SOLCVERSION_SHORT=$(echo "$SOLCVERSION" | sed -En 's/^([0-9.]+).*\+commit\.[0-9a-f]+.*$/\1/p')
    printLog "Using compiler version $SOLCVERSION"
}

function download_project
{
    local repo="$1"
    local solcjs_branch="$2"
    local test_dir="$3"

    printLog "Cloning $solcjs_branch of $repo..."
    git clone --depth 1 "$repo" -b "$solcjs_branch" "$test_dir/ext"
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
    printLog "Removing package lock files..."
    rm --force --verbose yarn.lock
    rm --force --verbose package-lock.json
}

function neutralize_package_json_hooks
{
    printLog "Disabling package.json hooks..."
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
    local binary_type="$2"
    local solc_path="$3"
    local level="$4"
    local evm_version="${5:-"$CURRENT_EVM_VERSION"}"

    [[ $binary_type == native || $binary_type == solcjs ]] || assertFail

    [[ $binary_type == native ]] && local solc_path="native"

    printLog "Forcing Truffle compiler settings..."
    echo "-------------------------------------"
    echo "Config file: $config_file"
    echo "Binary type: $binary_type"
    echo "Compiler path: $solc_path"
    echo "Optimization level: $level"
    echo "Optimizer settings: $(optimizer_settings_for_level "$level")"
    echo "EVM version: $evm_version"
    echo "-------------------------------------"

    # Forcing the settings should always work by just overwriting the solc object. Forcing them by using a
    # dedicated settings objects should only be the fallback.
    echo "module.exports['compilers'] = $(truffle_compiler_settings "$solc_path" "$level" "$evm_version");" >> "$config_file"
}

function force_hardhat_compiler_binary
{
    local config_file="$1"
    local binary_type="$2"
    local solc_path="$3"

    printLog "Configuring Hardhat..."
    echo "-------------------------------------"
    echo "Config file: ${config_file}"
    echo "Binary type: ${binary_type}"
    echo "Compiler path: ${solc_path}"
    hardhat_solc_build_subtask "$SOLCVERSION_SHORT" "$SOLCVERSION" "$binary_type" "$solc_path" >> "$config_file"
}

function force_hardhat_compiler_settings
{
    local config_file="$1"
    local level="$2"
    local evm_version="${3:-"$CURRENT_EVM_VERSION"}"

    printLog "Configuring Hardhat..."
    echo "-------------------------------------"
    echo "Config file: ${config_file}"
    echo "Optimization level: ${level}"
    echo "Optimizer settings: $(optimizer_settings_for_level "$level")"
    echo "EVM version: ${evm_version}"
    echo "Compiler version: ${SOLCVERSION_SHORT}"
    echo "Compiler version (full): ${SOLCVERSION}"
    echo "-------------------------------------"

    {
        echo -n 'module.exports["solidity"] = '
        hardhat_compiler_settings "$SOLCVERSION_SHORT" "$level" "$evm_version"
    } >> "$config_file"
}

function truffle_verify_compiler_version
{
    local solc_version="$1"
    local full_solc_version="$2"

    printLog "Verify that the correct version (${solc_version}/${full_solc_version}) of the compiler was used to compile the contracts..."
    grep "$full_solc_version" --with-filename --recursive build/contracts || fail "Wrong compiler version detected."
}

function hardhat_verify_compiler_version
{
    local solc_version="$1"
    local full_solc_version="$2"

    printLog "Verify that the correct version (${solc_version}/${full_solc_version}) of the compiler was used to compile the contracts..."
    grep '"solcVersion": "'"${solc_version}"'"' --with-filename artifacts/build-info/*.json || fail "Wrong compiler version detected."
    grep '"solcLongVersion": "'"${full_solc_version}"'"' --with-filename artifacts/build-info/*.json || fail "Wrong compiler version detected."
}

function truffle_clean
{
    rm -rf build/
}

function hardhat_clean
{
    rm -rf artifacts/ cache/
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
        2) echo "{enabled: true, details: {yul: false}}" ;;
        3) echo "{enabled: true, details: {yul: true}}" ;;
        *)
            fail "Optimizer level not found. Please define OPTIMIZER_LEVEL=[1, 2, 3]"
            ;;
    esac
}

function replace_global_solc
{
    local solc_path="$1"

    [[ ! -e solc ]] || fail "A file named 'solc' already exists in '${PWD}'."

    ln -s "$solc_path" solc
    export PATH="$PWD:$PATH"
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

function hardhat_solc_build_subtask {
    local solc_version="$1"
    local full_solc_version="$2"
    local binary_type="$3"
    local solc_path="$4"

    [[ $binary_type == native || $binary_type == solcjs ]] || assertFail

    [[ $binary_type == native ]] && local is_solcjs=false
    [[ $binary_type == solcjs ]] && local is_solcjs=true

    echo "const {TASK_COMPILE_SOLIDITY_GET_SOLC_BUILD} = require('hardhat/builtin-tasks/task-names');"
    echo "const assert = require('assert');"
    echo
    echo "subtask(TASK_COMPILE_SOLIDITY_GET_SOLC_BUILD, async (args, hre, runSuper) => {"
    echo "    assert(args.solcVersion == '${solc_version}', 'Unexpected solc version: ' + args.solcVersion)"
    echo "    return {"
    echo "        compilerPath: '$(realpath "$solc_path")',"
    echo "        isSolcJs: ${is_solcjs},"
    echo "        version: args.solcVersion,"
    echo "        longVersion: '${full_solc_version}'"
    echo "    }"
    echo "})"
}

function hardhat_compiler_settings {
    local solc_version="$1"
    local level="$2"
    local evm_version="$3"

    echo "{"
    echo "    version: '${solc_version}',"
    echo "    settings: {"
    echo "        optimizer: $(optimizer_settings_for_level "$level"),"
    echo "        evmVersion: '${evm_version}'"
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
    local binary_type="$2"
    local solc_path="$3"
    local optimizer_level="$4"
    local compile_fn="$5"
    local test_fn="$6"

    truffle_clean
    force_truffle_compiler_settings "$config_file" "$binary_type" "$solc_path" "$optimizer_level"
    compile_and_run_test compile_fn test_fn truffle_verify_compiler_version
}

function hardhat_run_test
{
    local config_file="$1"
    local optimizer_level="$2"
    local compile_fn="$3"
    local test_fn="$4"

    hardhat_clean
    force_hardhat_compiler_settings "$config_file" "$optimizer_level"
    compile_and_run_test compile_fn test_fn hardhat_verify_compiler_version
}

function external_test
{
    local name="$1"
    local main_fn="$2"

    printTask "Testing $name..."
    echo "==========================="
    DIR=$(mktemp -d -t "ext-test-${name}-XXXXXX")
    (
        [[ "$main_fn" != "" ]] || fail "Test main function not defined."
        $main_fn
    )
    rm -rf "$DIR"
    echo "Done."
}
