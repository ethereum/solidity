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

function compile_fn { yarn run build; }
function test_fn { yarn test; }

function gp2_test
{
    local repo="https://github.com/cowprotocol/contracts.git"
    local ref_type=branch
    local ref=main
    local config_file="hardhat.config.ts"
    local config_var="config"

    local compile_only_presets=(
        legacy-no-optimize        # Tests doing `new GPv2VaultRelayer` fail with "Error: Transaction reverted: trying to deploy a contract whose code is too large"
    )
    local settings_presets=(
        "${compile_only_presets[@]}"
        #ir-no-optimize           # Compilation fails with "YulException: Variable var_amount_1468 is 10 slot(s) too deep inside the stack."
        #ir-no-optimize           # Compilation fails with "YulException: Variable var_offset_3451 is 1 slot(s) too deep inside the stack."
        ir-optimize-evm+yul
        legacy-optimize-evm-only
        legacy-optimize-evm+yul
    )

    [[ $SELECTED_PRESETS != "" ]] || SELECTED_PRESETS=$(circleci_select_steps_multiarg "${settings_presets[@]}")
    print_presets_or_exit "$SELECTED_PRESETS"

    setup_solc "$DIR" "$BINARY_TYPE" "$BINARY_PATH"
    download_project "$repo" "$ref_type" "$ref" "$DIR"
    [[ $BINARY_TYPE == native ]] && replace_global_solc "$BINARY_PATH"

    neutralize_package_lock
    neutralize_package_json_hooks
    name_hardhat_default_export "$config_file" "$config_var"
    force_hardhat_compiler_binary "$config_file" "$BINARY_TYPE" "$BINARY_PATH"
    force_hardhat_compiler_settings "$config_file" "$(first_word "$SELECTED_PRESETS")" "$config_var"
    force_hardhat_unlimited_contract_size "$config_file" "$config_var"
    yarn

    # New hardhat release breaks GP2 tests, and since GP2 repository has been archived, we are pinning hardhat
    # to the previous stable version. See https://github.com/ethereum/solidity/pull/13485
    yarn add hardhat@2.10.2
    # hardhat-tenderly@1.2.0 and upwards break the build, hence we are pinning the version to the last stable one.
    # See https://github.com/cowprotocol/contracts/issues/32
    yarn add @tenderly/hardhat-tenderly@1.1.6

    # Some dependencies come with pre-built artifacts. We want to build from scratch.
    rm -r node_modules/@gnosis.pm/safe-contracts/build/

    # FIXME: One of the E2E tests tries to import artifacts from Gnosis Safe. We should rebuild them
    # but it's not that easy because @gnosis.pm/safe-contracts does not come with Hardhat config.
    rm test/e2e/contractOrdersWithGnosisSafe.test.ts

    # Patch contracts for 0.8.x compatibility.
    # NOTE: I'm patching OpenZeppelin as well instead of installing OZ 4.0 because it requires less
    # work. The project imports files that were moved to different locations in 4.0.
    sed -i 's|uint256(-1)|type(uint256).max|g' src/contracts/GPv2Settlement.sol
    sed -i 's|return msg\.sender;|return payable(msg.sender);|g' node_modules/@openzeppelin/contracts/utils/Context.sol
    perl -i -0pe \
        "s/uint256 (executedBuyAmount = \(-tokenDeltas\[trade.buyTokenIndex\]\)\n\s+.toUint256\(\);)/uint256 executedBuyAmount; unchecked \{\1\}/g" \
        src/contracts/GPv2Settlement.sol

    # This test is not supposed to work. The compiler is supposed to enforce zero padding since
    # at least 0.5.8 (see https://github.com/ethereum/solidity/pull/5815). For some reason the
    # test worked on 0.7.6 but no longer works on 0.8.x.
    sed -i 's|it\(("invalid EVM transaction encoding does not change order hash"\)|it.skip\1|g' test/GPv2Signing.test.ts

    # Disable tests that won't pass on the ir presets due to Hardhat heuristics. Note that this also disables
    # them for other presets but that's fine - we want same code run for benchmarks to be comparable.
    # TODO: Remove this when Hardhat adjusts heuristics for IR (https://github.com/nomiclabs/hardhat/issues/2115).
    sed -i 's|it\(("should revert when encoding invalid flags"\)|it.skip\1|g' test/GPv2Trade.test.ts

    replace_version_pragmas

    for preset in $SELECTED_PRESETS; do
        hardhat_run_test "$config_file" "$preset" "${compile_only_presets[*]}" compile_fn test_fn "$config_var"
        store_benchmark_report hardhat gp2 "$repo" "$preset"
    done
}

external_test Gnosis-Protocol-V2 gp2_test
