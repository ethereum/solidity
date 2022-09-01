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
# (c) 2021 solidity contributors.
#------------------------------------------------------------------------------

set -e

source scripts/common.sh
source test/externalTests/common.sh

REPO_ROOT=$(realpath "$(dirname "$0")/../..")

verify_input "$@"
BINARY_TYPE="$1"
BINARY_PATH="$(realpath "$2")"
SELECTED_PRESETS="$3"

function compile_fn { yarn build; }

function test_fn {
    # shellcheck disable=SC2046
    TS_NODE_TRANSPILE_ONLY=1 npx hardhat test --no-compile $(
        # TODO: We need to skip Migration.test.ts because it fails and makes other tests fail too.
        # Replace this with `yarn test` once https://github.com/sushiswap/trident/issues/283 is fixed.
        find test/ -name "*.test.ts" ! -path "test/Migration.test.ts" | LC_ALL=C sort
    )
}

function trident_test
{
    local repo="https://github.com/sushiswap/trident"
    local ref_type=commit
    # FIXME: Switch back to master branch when https://github.com/sushiswap/trident/issues/303 gets fixed.
    local ref="0cab5ae884cc9a41223d52791be775c3a053cb26" # master as of 2021-12-16
    local config_file="hardhat.config.ts"
    local config_var=config

    local compile_only_presets=()
    local settings_presets=(
        "${compile_only_presets[@]}"
        #ir-no-optimize            # Compilation fails with: "YulException: Variable var_amount_165 is 9 slot(s) too deep inside the stack."
        #ir-optimize-evm-only      # Compilation fails with: "YulException: Variable var_amount_165 is 9 slot(s) too deep inside the stack."
        ir-optimize-evm+yul       # Needs memory-safe inline assembly patch
        legacy-no-optimize
        legacy-optimize-evm-only
        legacy-optimize-evm+yul
    )

    [[ $SELECTED_PRESETS != "" ]] || SELECTED_PRESETS=$(circleci_select_steps_multiarg "${settings_presets[@]}")
    print_presets_or_exit "$SELECTED_PRESETS"

    setup_solc "$DIR" "$BINARY_TYPE" "$BINARY_PATH"
    download_project "$repo" "$ref_type" "$ref" "$DIR"

    # TODO: Currently tests work only with the exact versions from yarn.lock.
    # Re-enable this when https://github.com/sushiswap/trident/issues/284 is fixed.
    #neutralize_package_lock

    neutralize_package_json_hooks
    force_hardhat_compiler_binary "$config_file" "$BINARY_TYPE" "$BINARY_PATH"
    force_hardhat_compiler_settings "$config_file" "$(first_word "$SELECTED_PRESETS")" "$config_var"
    yarn install

    replace_version_pragmas
    force_solc_modules "${DIR}/solc"

    # BentoBoxV1Flat.sol requires a few small tweaks to compile on 0.8.x.
    # TODO: Remove once https://github.com/sushiswap/trident/pull/282 gets merged.
    sed -i 's|uint128(-1)|type(uint128).max|g' contracts/flat/BentoBoxV1Flat.sol
    sed -i 's|uint64(-1)|type(uint64).max|g' contracts/flat/BentoBoxV1Flat.sol
    sed -i 's|uint32(-1)|type(uint32).max|g' contracts/flat/BentoBoxV1Flat.sol
    sed -i 's|IERC20(0)|IERC20(address(0))|g' contracts/flat/BentoBoxV1Flat.sol
    sed -i 's|IStrategy(0)|IStrategy(address(0))|g' contracts/flat/BentoBoxV1Flat.sol
    find contracts -name "*.sol" -exec sed -i -e 's/^\(\s*\)\(assembly\)/\1\/\/\/ @solidity memory-safe-assembly\n\1\2/' '{}' \;

    # TODO: Remove this when https://github.com/NomicFoundation/hardhat/issues/2453 gets fixed.
    sed -i 's|it\(("Reverts on direct deployment via factory"\)|it.skip\1|g' test/MasterDeployer.test.ts

    # @sushiswap/core package contains contracts that get built with 0.6.12 and fail our compiler
    # version check. It's not used by tests so we can remove it.
    rm -r node_modules/@sushiswap/core/

    for preset in $SELECTED_PRESETS; do
        hardhat_run_test "$config_file" "$preset" "${compile_only_presets[*]}" compile_fn test_fn "$config_var"
        store_benchmark_report hardhat trident "$repo" "$preset"
    done
}

external_test Trident trident_test
