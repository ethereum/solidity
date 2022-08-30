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
# (c) 2022 solidity contributors.
#------------------------------------------------------------------------------

set -e

source scripts/common.sh
source test/externalTests/common.sh

REPO_ROOT=$(realpath "$(dirname "$0")/../..")

verify_input "$@"
BINARY_TYPE="$1"
BINARY_PATH="$(realpath "$2")"
SELECTED_PRESETS="$3"

function compile_fn { yarn compile; }
function test_fn { SNAPSHOT_UPDATE=1 npx --no hardhat test; }

function brink_test
{
    local repo="https://github.com/brinktrade/brink-core"
    local ref_type=branch
    local ref=master
    local config_file="hardhat.config.js"
    local config_var=""
    local extra_settings="metadata: {bytecodeHash: 'none'}"
    local extra_optimizer_settings="runs: 800"

    local compile_only_presets=(
        #ir-no-optimize            # Compilation fails with "YulException: Variable var_signature_127_offset is 2 slot(s) too deep inside the stack."
        #ir-optimize-evm-only      # Compilation fails with "YulException: Variable var_signature_127_offset is 2 slot(s) too deep inside the stack."
        ir-optimize-evm+yul        # Lots of test failures. Tests depend on constants.js, which seems to be calculated specifically for 0.8.10.
        legacy-optimize-evm+yul    # Lots of test failures. Tests depend on constants.js, which seems to be calculated specifically for 0.8.10.
        legacy-no-optimize         # Lots of test failures. Tests depend on constants.js, which seems to be calculated specifically for 0.8.10.
        legacy-optimize-evm-only   # Lots of test failures. Tests depend on constants.js, which seems to be calculated specifically for 0.8.10.
    )
    local settings_presets=(
        "${compile_only_presets[@]}"
    )

    [[ $SELECTED_PRESETS != "" ]] || SELECTED_PRESETS=$(circleci_select_steps_multiarg "${settings_presets[@]}")
    print_presets_or_exit "$SELECTED_PRESETS"

    setup_solc "$DIR" "$BINARY_TYPE" "$BINARY_PATH"
    download_project "$repo" "$ref_type" "$ref" "$DIR"

    # TODO: Remove this when Brink merges https://github.com/brinktrade/brink-core/pull/52
    sed -i "s|\(function isValidSignature(bytes \)calldata\( _data, bytes \)calldata\( _signature)\)|\1memory\2memory\3|g" contracts/Test/MockEIP1271Validator.sol

    neutralize_package_lock
    neutralize_package_json_hooks
    force_hardhat_compiler_binary "$config_file" "$BINARY_TYPE" "$BINARY_PATH"
    force_hardhat_compiler_settings "$config_file" "$(first_word "$SELECTED_PRESETS")" "$config_var" "$CURRENT_EVM_VERSION" "$extra_settings" "$extra_optimizer_settings"
    yarn install
    yarn add hardhat-gas-reporter

    # TODO: Remove when https://github.com/brinktrade/brink-core/issues/48 is fixed.
    yarn add chai

    replace_version_pragmas

    for preset in $SELECTED_PRESETS; do
        hardhat_run_test "$config_file" "$preset" "${compile_only_presets[*]}" compile_fn test_fn "$config_var" "$extra_settings" "$extra_optimizer_settings"
        store_benchmark_report hardhat brink "$repo" "$preset"
    done
}

external_test Brink brink_test
