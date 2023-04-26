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

# Requires $REPO_ROOT to be defined and "${REPO_ROOT}/scripts/common.sh" to be included before.

CURRENT_EVM_VERSION=shanghai

AVAILABLE_PRESETS=(
    legacy-no-optimize
    ir-no-optimize
    legacy-optimize-evm-only
    ir-optimize-evm-only
    legacy-optimize-evm+yul
    ir-optimize-evm+yul
)

function print_presets_or_exit
{
    local selected_presets="$1"

    [[ $selected_presets != "" ]] || { printWarning "No presets to run. Exiting."; exit 0; }

    printLog "Selected settings presets: ${selected_presets}"
}

function verify_input
{
    local binary_type="$1"
    local binary_path="$2"
    local selected_presets="$3"

    (( $# >= 2 && $# <= 3 )) || fail "Usage: $0 native|solcjs <path to solc or soljson.js> [preset]"
    [[ $binary_type == native || $binary_type == solcjs ]] || fail "Invalid binary type: '${binary_type}'. Must be either 'native' or 'solcjs'."
    [[ -f "$binary_path" ]] || fail "The compiler binary does not exist at '${binary_path}'"

    if [[ $selected_presets != "" ]]
    then
        for preset in $selected_presets
        do
            if [[ " ${AVAILABLE_PRESETS[*]} " != *" $preset "* ]]
            then
                fail "Preset '${preset}' does not exist. Available presets: ${AVAILABLE_PRESETS[*]}."
            fi
        done
    fi
}

function setup_solc
{
    local test_dir="$1"
    local binary_type="$2"
    local binary_path="$3"
    local solcjs_branch="${4:-master}"
    local install_dir="${5:-solc/}"
    local solcjs_dir="$6"

    [[ $binary_type == native || $binary_type == solcjs ]] || assertFail
    [[ $binary_type == solcjs || $solcjs_dir == "" ]] || assertFail

    cd "$test_dir"

    if [[ $binary_type == solcjs ]]
    then
        printLog "Setting up solc-js..."
        if [[ $solcjs_dir == "" ]]; then
            printLog "Cloning branch ${solcjs_branch}..."
            git clone --depth 1 -b "$solcjs_branch" https://github.com/ethereum/solc-js.git "$install_dir"
        else
            printLog "Using local solc-js from ${solcjs_dir}..."
            cp -ra "$solcjs_dir" solc
        fi

        pushd "$install_dir"
        npm install
        cp "$binary_path" soljson.js
        npm run build
        SOLCVERSION=$(dist/solc.js --version)
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
    local ref_type="$2"
    local solcjs_ref="$3"
    local test_dir="$4"

    [[ $ref_type == commit || $ref_type == branch || $ref_type == tag ]] || assertFail

    printLog "Cloning ${ref_type} ${solcjs_ref} of ${repo}..."
    if [[ $ref_type == commit ]]; then
        mkdir ext
        cd ext
        git init
        git remote add origin "$repo"
        git fetch --depth 1 origin "$solcjs_ref"
        git reset --hard FETCH_HEAD
    else
        git clone --depth 1 "$repo" -b "$solcjs_ref" "$test_dir/ext"
        cd ext
    fi
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

function neutralize_packaged_contracts
{
    # Frameworks will build contracts from any package that contains a configuration file.
    # This is both unnecessary (any files imported from these packages will get compiled again as a
    # part of the main project anyway) and trips up our version check because it won't use our
    # custom compiler binary.
    printLog "Removing framework config and artifacts from npm packages..."
    find node_modules/ -type f '(' -name 'hardhat.config.*' -o -name 'truffle-config.*' ')' -delete

    # Some npm packages also come packaged with pre-built artifacts.
    find node_modules/ -path '*artifacts/build-info/*.json' -delete
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
    local preset="$4"
    local evm_version="${5:-"$CURRENT_EVM_VERSION"}"
    local extra_settings="$6"
    local extra_optimizer_settings="$7"

    [[ $binary_type == native || $binary_type == solcjs ]] || assertFail

    [[ $binary_type == native ]] && local solc_path="native"

    printLog "Forcing Truffle compiler settings..."
    echo "-------------------------------------"
    echo "Config file: $config_file"
    echo "Binary type: $binary_type"
    echo "Compiler path: $solc_path"
    echo "Settings preset: ${preset}"
    echo "Settings: $(settings_from_preset "$preset" "$evm_version" "$extra_settings" "$extra_optimizer_settings")"
    echo "EVM version: $evm_version"
    echo "Compiler version: ${SOLCVERSION_SHORT}"
    echo "Compiler version (full): ${SOLCVERSION}"
    echo "-------------------------------------"

    local compiler_settings gas_reporter_settings
    compiler_settings=$(truffle_compiler_settings "$solc_path" "$preset" "$evm_version" "$extra_settings" "$extra_optimizer_settings")
    gas_reporter_settings=$(eth_gas_reporter_settings "$preset")

    {
        echo "require('eth-gas-reporter');"
        echo "module.exports['mocha'] = {"
        echo "    reporter: 'eth-gas-reporter',"
        echo "    reporterOptions: ${gas_reporter_settings}"
        echo "};"

        echo "module.exports['compilers'] = ${compiler_settings};"
    } >> "$config_file"
}

function name_hardhat_default_export
{
    local config_file="$1"
    local import="import {HardhatUserConfig} from 'hardhat/types/config';"
    local config="const config: HardhatUserConfig = {"
    sed -i "s|^\s*export\s*default\s*{|${import}\n${config}|g" "$config_file"
    echo "export default config;" >> "$config_file"
}

function force_hardhat_timeout
{
    local config_file="$1"
    local config_var_name="$2"
    local new_timeout="$3"

    printLog "Configuring Hardhat..."
    echo "-------------------------------------"
    echo "Timeout: ${new_timeout}"
    echo "-------------------------------------"

    if [[ $config_file == *\.js ]]; then
        [[ $config_var_name == "" ]] || assertFail
        echo "module.exports.mocha = module.exports.mocha || {timeout: ${new_timeout}}"
        echo "module.exports.mocha.timeout =  ${new_timeout}"
    else
        [[ $config_file == *\.ts ]] || assertFail
        [[ $config_var_name != "" ]] || assertFail
        echo "${config_var_name}.mocha = ${config_var_name}.mocha ?? {timeout: ${new_timeout}};"
        echo "${config_var_name}.mocha!.timeout = ${new_timeout}"
    fi >> "$config_file"
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

    local language="${config_file##*.}"
    hardhat_solc_build_subtask "$SOLCVERSION_SHORT" "$SOLCVERSION" "$binary_type" "$solc_path" "$language" >> "$config_file"
}

function force_hardhat_unlimited_contract_size
{
    local config_file="$1"
    local config_var_name="$2"

    printLog "Configuring Hardhat..."
    echo "-------------------------------------"
    echo "Allow unlimited contract size: true"
    echo "-------------------------------------"

    if [[ $config_file == *\.js ]]; then
        [[ $config_var_name == "" ]] || assertFail
        echo "module.exports.networks.hardhat = module.exports.networks.hardhat || {allowUnlimitedContractSize: undefined}"
        echo "module.exports.networks.hardhat.allowUnlimitedContractSize = true"
    else
        [[ $config_file == *\.ts ]] || assertFail
        [[ $config_var_name != "" ]] || assertFail
        echo "${config_var_name}.networks!.hardhat = ${config_var_name}.networks!.hardhat ?? {allowUnlimitedContractSize: undefined};"
        echo "${config_var_name}.networks!.hardhat!.allowUnlimitedContractSize = true"
    fi >> "$config_file"
}

function force_hardhat_compiler_settings
{
    local config_file="$1"
    local preset="$2"
    local config_var_name="$3"
    local evm_version="${4:-"$CURRENT_EVM_VERSION"}"
    local extra_settings="$5"
    local extra_optimizer_settings="$6"

    printLog "Configuring Hardhat..."
    echo "-------------------------------------"
    echo "Config file: ${config_file}"
    echo "Settings preset: ${preset}"
    echo "Settings: $(settings_from_preset "$preset" "$evm_version" "$extra_settings" "$extra_optimizer_settings")"
    echo "EVM version: ${evm_version}"
    echo "Compiler version: ${SOLCVERSION_SHORT}"
    echo "Compiler version (full): ${SOLCVERSION}"
    echo "-------------------------------------"

    local compiler_settings gas_reporter_settings
    compiler_settings=$(hardhat_compiler_settings "$SOLCVERSION_SHORT" "$preset" "$evm_version" "$extra_settings" "$extra_optimizer_settings")
    gas_reporter_settings=$(eth_gas_reporter_settings "$preset")
    if [[ $config_file == *\.js ]]; then
        [[ $config_var_name == "" ]] || assertFail
        echo "require('hardhat-gas-reporter');"
        echo "module.exports.gasReporter = ${gas_reporter_settings};"
        echo "module.exports.solidity = ${compiler_settings};"
    else
        [[ $config_file == *\.ts ]] || assertFail
        [[ $config_var_name != "" ]] || assertFail
        echo 'import "hardhat-gas-reporter";'
        echo "${config_var_name}.gasReporter = ${gas_reporter_settings};"
        echo "${config_var_name}.solidity = {compilers: [${compiler_settings}]};"
    fi >> "$config_file"
}

function truffle_verify_compiler_version
{
    local solc_version="$1"
    local full_solc_version="$2"

    printLog "Verify that the correct version (${solc_version}/${full_solc_version}) of the compiler was used to compile the contracts..."
    grep "$full_solc_version" --recursive --quiet build/contracts || fail "Wrong compiler version detected."
}

function hardhat_verify_compiler_version
{
    local solc_version="$1"
    local full_solc_version="$2"

    printLog "Verify that the correct version (${solc_version}/${full_solc_version}) of the compiler was used to compile the contracts..."
    local build_info_files
    build_info_files=$(find . -path '*artifacts/build-info/*.json')
    for build_info_file in $build_info_files; do
        grep '"solcVersion":[[:blank:]]*"'"${solc_version}"'"' --quiet "$build_info_file" || fail "Wrong compiler version detected in ${build_info_file}."
        grep '"solcLongVersion":[[:blank:]]*"'"${full_solc_version}"'"' --quiet "$build_info_file" || fail "Wrong compiler version detected in ${build_info_file}."
    done
}

function truffle_clean
{
    rm -rf build/
}

function hardhat_clean
{
    rm -rf build/ artifacts/ cache/
}

function settings_from_preset
{
    local preset="$1"
    local evm_version="$2"
    local extra_settings="$3"
    local extra_optimizer_settings="$4"

    [[ " ${AVAILABLE_PRESETS[*]} " == *" $preset "* ]] || assertFail

    [[ $extra_settings == "" ]] || extra_settings="${extra_settings}, "
    [[ $extra_optimizer_settings == "" ]] || extra_optimizer_settings="${extra_optimizer_settings}, "

    case "$preset" in
        # NOTE: Remember to update `parallelism` of `t_ems_ext` job in CI config if you add/remove presets
        legacy-no-optimize)       echo "{${extra_settings}evmVersion: '${evm_version}', viaIR: false, optimizer: {${extra_optimizer_settings}enabled: false}}" ;;
        ir-no-optimize)           echo "{${extra_settings}evmVersion: '${evm_version}', viaIR: true,  optimizer: {${extra_optimizer_settings}enabled: false}}" ;;
        legacy-optimize-evm-only) echo "{${extra_settings}evmVersion: '${evm_version}', viaIR: false, optimizer: {${extra_optimizer_settings}enabled: true, details: {yul: false}}}" ;;
        ir-optimize-evm-only)     echo "{${extra_settings}evmVersion: '${evm_version}', viaIR: true,  optimizer: {${extra_optimizer_settings}enabled: true, details: {yul: false}}}" ;;
        legacy-optimize-evm+yul)  echo "{${extra_settings}evmVersion: '${evm_version}', viaIR: false, optimizer: {${extra_optimizer_settings}enabled: true, details: {yul: true}}}" ;;
        ir-optimize-evm+yul)      echo "{${extra_settings}evmVersion: '${evm_version}', viaIR: true,  optimizer: {${extra_optimizer_settings}enabled: true, details: {yul: true}}}" ;;
        *)
            fail "Unknown settings preset: '${preset}'."
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

function eth_gas_reporter_settings
{
    local preset="$1"

    echo "{"
    echo "    enabled: true,"
    echo "    gasPrice: 1,"                           # Gas price does not matter to us at all. Set to whatever to avoid API call.
    echo "    noColors: true,"
    echo "    showTimeSpent: false,"                  # We're not interested in test timing
    echo "    onlyCalledMethods: true,"               # Exclude entries with no gas for shorter report
    echo "    showMethodSig: true,"                   # Should make diffs more stable if there are overloaded functions
    echo "    outputFile: \"$(gas_report_path "$preset")\""
    echo "}"
}

function truffle_compiler_settings
{
    local solc_path="$1"
    local preset="$2"
    local evm_version="$3"
    local extra_settings="$4"
    local extra_optimizer_settings="$5"

    echo "{"
    echo "    solc: {"
    echo "        version: \"${solc_path}\","
    echo "        settings: $(settings_from_preset "$preset" "$evm_version" "$extra_settings" "$extra_optimizer_settings")"
    echo "    }"
    echo "}"
}

function hardhat_solc_build_subtask {
    local solc_version="$1"
    local full_solc_version="$2"
    local binary_type="$3"
    local solc_path="$4"
    local language="$5"

    [[ $binary_type == native || $binary_type == solcjs ]] || assertFail

    [[ $binary_type == native ]] && local is_solcjs=false
    [[ $binary_type == solcjs ]] && local is_solcjs=true

    if [[ $language == js ]]; then
        echo "const {TASK_COMPILE_SOLIDITY_GET_SOLC_BUILD} = require('hardhat/builtin-tasks/task-names');"
        echo "const assert = require('assert');"
        echo
        echo "subtask(TASK_COMPILE_SOLIDITY_GET_SOLC_BUILD, async (args, hre, runSuper) => {"
    else
        [[ $language == ts ]] || assertFail
        echo "import {TASK_COMPILE_SOLIDITY_GET_SOLC_BUILD} from 'hardhat/builtin-tasks/task-names';"
        echo "import assert = require('assert');"
        echo "import {subtask} from 'hardhat/config';"
        echo
        echo "subtask(TASK_COMPILE_SOLIDITY_GET_SOLC_BUILD, async (args: any, _hre: any, _runSuper: any) => {"
    fi

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
    local preset="$2"
    local evm_version="$3"
    local extra_settings="$4"
    local extra_optimizer_settings="$5"

    echo "{"
    echo "    version: '${solc_version}',"
    echo "    settings: $(settings_from_preset "$preset" "$evm_version" "$extra_settings" "$extra_optimizer_settings")"
    echo "}"
}

function compile_and_run_test
{
    local compile_fn="$1"
    local test_fn="$2"
    local verify_fn="$3"
    local preset="$4"
    local compile_only_presets="$5"

    [[ $preset != *" "* ]] || assertFail "Preset names must not contain spaces."

    printLog "Running compile function..."
    time $compile_fn
    $verify_fn "$SOLCVERSION_SHORT" "$SOLCVERSION"

    if [[ "$COMPILE_ONLY" == 1 || " $compile_only_presets " == *" $preset "* ]]; then
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
    local preset="$4"
    local compile_only_presets="$5"
    local compile_fn="$6"
    local test_fn="$7"
    local extra_settings="$8"
    local extra_optimizer_settings="$9"

    truffle_clean
    force_truffle_compiler_settings "$config_file" "$binary_type" "$solc_path" "$preset" "$CURRENT_EVM_VERSION" "$extra_settings" "$extra_optimizer_settings"
    compile_and_run_test compile_fn test_fn truffle_verify_compiler_version "$preset" "$compile_only_presets"
}

function hardhat_run_test
{
    local config_file="$1"
    local preset="$2"
    local compile_only_presets="$3"
    local compile_fn="$4"
    local test_fn="$5"
    local config_var_name="$6"
    local extra_settings="$7"
    local extra_optimizer_settings="$8"

    hardhat_clean
    force_hardhat_compiler_settings "$config_file" "$preset" "$config_var_name" "$CURRENT_EVM_VERSION" "$extra_settings" "$extra_optimizer_settings"
    compile_and_run_test compile_fn test_fn hardhat_verify_compiler_version "$preset" "$compile_only_presets"
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

function gas_report_path
{
    local preset="$1"

    echo "${DIR}/gas-report-${preset}.rst"
}

function gas_report_to_json
{
    cat - | "${REPO_ROOT}/scripts/externalTests/parse_eth_gas_report.py" | jq '{gas: .}'
}

function detect_hardhat_artifact_dir
{
    if [[ -e build/ && -e artifacts/ ]]; then
        fail "Cannot determine Hardhat artifact location. Both build/ and artifacts/ exist"
    elif [[ -e build/ ]]; then
        echo -n build/artifacts
    elif [[ -e artifacts/ ]]; then
        echo -n artifacts
    else
        fail "Hardhat build artifacts not found."
    fi
}

function bytecode_size_json_from_truffle_artifacts
{
    # NOTE: The output of this function is a series of concatenated JSON dicts rather than a list.

    for artifact in build/contracts/*.json; do
        if [[ $(jq '. | has("unlinked_binary")' "$artifact") == false ]]; then
            # Each artifact represents compilation output for a single contract. Some top-level keys contain
            # bits of Standard JSON output while others are generated by Truffle. Process it into a dict
            # of the form `{"<file>": {"<contract>": <size>}}`.
            # NOTE: The `bytecode` field starts with 0x, which is why we subtract 1 from size.
            jq '{
                (.ast.absolutePath): {
                    (.contractName): (.bytecode | length / 2 - 1)
                }
            }' "$artifact"
        fi
    done
}

function bytecode_size_json_from_hardhat_artifacts
{
    # NOTE: The output of this function is a series of concatenated JSON dicts rather than a list.

    for artifact in "$(detect_hardhat_artifact_dir)"/build-info/*.json; do
        # Each artifact contains Standard JSON output under the `output` key.
        # Process it into a dict of the form `{"<file>": {"<contract>": <size>}}`,
        # Note that one Hardhat artifact often represents multiple input files.
        jq '.output.contracts | to_entries[] | {
            "\(.key)": .value | to_entries[] | {
                "\(.key)": (.value.evm.bytecode.object | length / 2)
            }
        }' "$artifact"
    done
}

function combine_artifact_json
{
    # Combine all dicts into a list with `jq --slurp` and then use `reduce` to merge them into one
    # big dict with keys of the form `"<file>:<contract>"`. Then run jq again to filter out items
    # with zero size and put the rest under under a top-level `bytecode_size` key. Also add another
    # key with total bytecode size.
    # NOTE: The extra inner `bytecode_size` key is there only to make diffs more readable.
    cat - |
        jq --slurp 'reduce (.[] | to_entries[]) as {$key, $value} ({}; . + {
            ($key + ":" + ($value | to_entries[].key)): {
                bytecode_size: $value | to_entries[].value
            }
        })' |
        jq --indent 4 --sort-keys '{
            bytecode_size: [. | to_entries[] | select(.value.bytecode_size > 0)] | from_entries,
            total_bytecode_size: (reduce (. | to_entries[]) as {$key, $value} (0; . + $value.bytecode_size))
        }'
}

function project_info_json
{
    local project_url="$1"

    echo "{"
    echo "    \"project\": {"
    # NOTE: Given that we clone with `--depth 1`, we'll only get useful output out of `git describe`
    # if we directly check out a tag. Still better than nothing.
    echo "        \"version\": \"$(git describe --always)\","
    echo "        \"commit\": \"$(git rev-parse HEAD)\","
    echo "        \"url\": \"${project_url}\""
    echo "    }"
    echo "}"
}

function store_benchmark_report
{
    local framework="$1"
    local project_name="$2"
    local project_url="$3"
    local preset="$4"

    [[ $framework == truffle || $framework == hardhat ]] || assertFail
    [[ " ${AVAILABLE_PRESETS[*]} " == *" $preset "* ]] || assertFail

    local report_dir="${REPO_ROOT}/reports/externalTests"
    local output_file="${report_dir}/benchmark-${project_name}-${preset}.json"
    mkdir -p "$report_dir"

    {
        if [[ -e $(gas_report_path "$preset") ]]; then
            gas_report_to_json < "$(gas_report_path "$preset")"
        fi

        "bytecode_size_json_from_${framework}_artifacts" | combine_artifact_json
        project_info_json "$project_url"
    } | jq --slurp "{\"${project_name}\": {\"${preset}\": add}}" --indent 4 --sort-keys > "$output_file"
}
