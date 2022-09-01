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
function test_fn { UPDATE_SNAPSHOT=1 npx hardhat test; }

function uniswap_test
{
    local repo="https://github.com/solidity-external-tests/uniswap-v3-core.git"
    local ref_type=branch
    local ref=main_080
    local config_file="hardhat.config.ts"
    local config_var=config

    local compile_only_presets=()
    local settings_presets=(
        "${compile_only_presets[@]}"
        #ir-no-optimize           # Compilation fails with: "YulException: Variable ret_0 is 1 slot(s) too deep inside the stack."
        #ir-optimize-evm-only     # Compilation fails with: "YulException: Variable ret_0 is 1 slot(s) too deep inside the stack."
        ir-optimize-evm+yul
        legacy-no-optimize
        legacy-optimize-evm-only
        legacy-optimize-evm+yul
    )

    [[ $SELECTED_PRESETS != "" ]] || SELECTED_PRESETS=$(circleci_select_steps_multiarg "${settings_presets[@]}")
    print_presets_or_exit "$SELECTED_PRESETS"

    setup_solc "$DIR" "$BINARY_TYPE" "$BINARY_PATH"
    download_project "$repo" "$ref_type" "$ref" "$DIR"

    # Disable tests that won't pass on the ir presets due to Hardhat heuristics. Note that this also disables
    # them for other presets but that's fine - we want same code run for benchmarks to be comparable.
    # TODO: Remove this when https://github.com/NomicFoundation/hardhat/issues/2115 gets fixed.
    sed -i "s|it\(('underpay zero for one and exact in',\)|it.skip\1|g" test/UniswapV3Pool.spec.ts
    sed -i "s|it\(('pay in the wrong token zero for one and exact in',\)|it.skip\1|g" test/UniswapV3Pool.spec.ts
    sed -i "s|it\(('underpay zero for one and exact out',\)|it.skip\1|g" test/UniswapV3Pool.spec.ts
    sed -i "s|it\(('pay in the wrong token zero for one and exact out',\)|it.skip\1|g" test/UniswapV3Pool.spec.ts
    sed -i "s|it\(('underpay one for zero and exact in',\)|it.skip\1|g" test/UniswapV3Pool.spec.ts
    sed -i "s|it\(('pay in the wrong token one for zero and exact in',\)|it.skip\1|g" test/UniswapV3Pool.spec.ts
    sed -i "s|it\(('underpay one for zero and exact out',\)|it.skip\1|g" test/UniswapV3Pool.spec.ts
    sed -i "s|it\(('pay in the wrong token one for zero and exact out',\)|it.skip\1|g" test/UniswapV3Pool.spec.ts

    neutralize_package_json_hooks
    name_hardhat_default_export "$config_file" "$config_var"
    force_hardhat_compiler_binary "$config_file" "$BINARY_TYPE" "$BINARY_PATH"
    force_hardhat_compiler_settings "$config_file" "$(first_word "$SELECTED_PRESETS")" "$config_var"
    force_hardhat_unlimited_contract_size "$config_file" "$config_var"

    # Force latest versions of everything except Hardhat and some of its dependencies.
    # - Hardhat 2.2.0 pulled in by yarn.lock is too old because it can't decode Panics caused by overflow/underflow.
    # - Hardhat 2.5.0+ won't work because of error messages hard-coded in test expectations. E.g.
    #   "revert SPL" is expected but the message is "reverted with reason string 'SPL'" in 2.5.0.
    # - Newer versions of ethereumjs/tx have an issue with 'gteHardfork()' method.
    neutralize_package_lock
    yarn add hardhat@2.4.3
    yarn add @ethereumjs/tx@3.1.3

    yarn install
    yarn add hardhat-gas-reporter

    # With ethers.js 5.6.2 many tests for revert messages fail.
    # TODO: Remove when https://github.com/ethers-io/ethers.js/discussions/2849 is resolved.
    yarn add ethers@5.6.1

    replace_version_pragmas

    for preset in $SELECTED_PRESETS; do
        hardhat_run_test "$config_file" "$preset" "${compile_only_presets[*]}" compile_fn test_fn "$config_var"
        store_benchmark_report hardhat uniswap "$repo" "$preset"
    done
}

external_test Uniswap-V3 uniswap_test
