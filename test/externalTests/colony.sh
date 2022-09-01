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
BINARY_PATH="$(realpath "$2")"
SELECTED_PRESETS="$3"

function compile_fn { yarn run provision:token:contracts; }
function test_fn { yarn run test:contracts; }

function colony_test
{
    local repo="https://github.com/solidity-external-tests/colonyNetwork.git"
    local ref_type=branch
    local ref="develop_080"
    local config_file="truffle.js"

    local compile_only_presets=(
        ir-no-optimize            # Compiles but tests run out of gas
        ir-optimize-evm-only      # Compiles but tests run out of gas
        legacy-no-optimize        # Compiles but tests run out of gas
        legacy-optimize-evm-only  # Compiles but tests run out of gas
    )
    local settings_presets=(
        "${compile_only_presets[@]}"
        ir-optimize-evm+yul
        legacy-optimize-evm+yul
    )

    [[ $SELECTED_PRESETS != "" ]] || SELECTED_PRESETS=$(circleci_select_steps_multiarg "${settings_presets[@]}")
    print_presets_or_exit "$SELECTED_PRESETS"

    setup_solc "$DIR" "$BINARY_TYPE" "$BINARY_PATH"
    download_project "$repo" "$ref_type" "$ref" "$DIR"
    [[ $BINARY_TYPE == native ]] && replace_global_solc "$BINARY_PATH"

    neutralize_package_json_hooks
    force_truffle_compiler_settings "$config_file" "$BINARY_TYPE" "${DIR}/solc/dist" "$(first_word "$SELECTED_PRESETS")"
    yarn install
    git submodule update --init

    cd lib
    rm -Rf dappsys
    git clone https://github.com/solidity-external-tests/dappsys-monolithic.git -b master_080 dappsys
    cd ..

    replace_version_pragmas
    [[ $BINARY_TYPE == solcjs ]] && force_solc_modules "${DIR}/solc/dist"

    for preset in $SELECTED_PRESETS; do
        truffle_run_test "$config_file" "$BINARY_TYPE" "${DIR}/solc/dist" "$preset" "${compile_only_presets[*]}" compile_fn test_fn
        store_benchmark_report truffle colony "$repo" "$preset"
    done
}

external_test ColonyNetworks colony_test
