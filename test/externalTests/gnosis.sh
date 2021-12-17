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

source scripts/common.sh
source test/externalTests/common.sh

REPO_ROOT=$(realpath "$(dirname "$0")/../..")

verify_input "$@"
BINARY_TYPE="$1"
BINARY_PATH="$2"
SELECTED_PRESETS="$3"

function compile_fn { npx truffle compile; }
function test_fn { npm test; }

function gnosis_safe_test
{
    local repo="https://github.com/solidity-external-tests/safe-contracts.git"
    local ref_type=branch
    local ref="development_080"
    local config_file="truffle-config.js"

    local compile_only_presets=()
    local settings_presets=(
        "${compile_only_presets[@]}"
        #ir-no-optimize            # Compilation fails with "YulException: Variable var_call_430_mpos is 1 slot(s) too deep inside the stack."
        #ir-optimize-evm-only      # Compilation fails with "YulException: Variable var_call_430_mpos is 1 slot(s) too deep inside the stack."
        ir-optimize-evm+yul
        #legacy-no-optimize        # Compilation fails with "Stack too deep" error
        #legacy-optimize-evm-only  # Compilation fails with "Stack too deep" error
        legacy-optimize-evm+yul
    )

    [[ $SELECTED_PRESETS != "" ]] || SELECTED_PRESETS=$(circleci_select_steps_multiarg "${settings_presets[@]}")
    print_presets_or_exit "$SELECTED_PRESETS"

    setup_solc "$DIR" "$BINARY_TYPE" "$BINARY_PATH"
    download_project "$repo" "$ref_type" "$ref" "$DIR"
    [[ $BINARY_TYPE == native ]] && replace_global_solc "$BINARY_PATH"

    sed -i 's|github:gnosis/mock-contract#sol_0_5_0|github:solidity-external-tests/mock-contract#master_080|g' package.json

    neutralize_package_lock
    neutralize_package_json_hooks
    force_truffle_compiler_settings "$config_file" "$BINARY_TYPE" "${DIR}/solc" "$(first_word "$SELECTED_PRESETS")"
    npm install --package-lock
    npm install eth-gas-reporter

    replace_version_pragmas
    [[ $BINARY_TYPE == solcjs ]] && force_solc_modules "${DIR}/solc"

    for preset in $SELECTED_PRESETS; do
        truffle_run_test "$config_file" "$BINARY_TYPE" "${DIR}/solc" "$preset" "${compile_only_presets[*]}" compile_fn test_fn
        store_benchmark_report truffle gnosis "$repo" "$preset"
    done
}

external_test Gnosis-Safe gnosis_safe_test
