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

function compile_fn { npm run build; }
function test_fn { npm run test; }

function elementfi_test
{
    local repo="https://github.com/element-fi/elf-contracts"
    local ref_type=branch
    local ref=main
    local config_file="hardhat.config.ts"
    local config_var=config

    local compile_only_presets=(
        # ElementFi's test suite is hard-coded for mainnet forked via alchemy.io.
        # Locally we can only compile.
        #ir-no-optimize           # Compilation fails with "YulException: Variable var_amount_9311 is 10 slot(s) too deep inside the stack."
        #ir-optimize-evm-only     # Compilation fails with "YulException: Variable var_amount_9311 is 10 slot(s) too deep inside the stack."
        ir-optimize-evm+yul
        legacy-no-optimize
        legacy-optimize-evm-only
        legacy-optimize-evm+yul
    )
    local settings_presets=(
        "${compile_only_presets[@]}"
    )

    [[ $SELECTED_PRESETS != "" ]] || SELECTED_PRESETS=$(circleci_select_steps_multiarg "${settings_presets[@]}")
    print_presets_or_exit "$SELECTED_PRESETS"

    setup_solc "$DIR" "$BINARY_TYPE" "$BINARY_PATH"
    download_project "$repo" "$ref_type" "$ref" "$DIR"

    chmod +x scripts/load-balancer-contracts.sh
    scripts/load-balancer-contracts.sh

    # Balancer contracts require 0.7.x. Patch them for 0.8.x.
    pushd contracts/balancer-core-v2
    sed -i 's|uint256(address(this))|uint256(uint160(address(this)))|g' test/MockPoolFactory.sol
    sed -i 's|uint256(address(this))|uint256(uint160(address(this)))|g' vault/ProtocolFeesCollector.sol
    sed -i 's|uint256(address(this))|uint256(uint160(address(this)))|g' vault/VaultAuthorization.sol
    sed -i 's|msg\.sender\.sendValue(excess)|payable(msg.sender).sendValue(excess)|g' vault/AssetTransfersHandler.sol
    sed -i 's|msg\.sender\.transfer(wad)|payable(msg.sender).transfer(wad)|g' test/WETH.sol
    sed -i 's|int256(-amountsOut\[i\])|-int256(amountsOut[i])|g' test/MockVault.sol
    sed -i 's|int256(-amount)|-int256(amount)|g' vault/AssetManagers.sol
    sed -i 's|uint256(-1)|type(uint256).max|g' pools/BalancerPoolToken.sol
    sed -i 's|uint256(-1)|type(uint256).max|g' test/WETH.sol
    sed -i 's|IERC20(0)|IERC20(address(0))|g' pools/BasePool.sol
    sed -i 's|IERC20(0)|IERC20(address(0))|g' vault/balances/TwoTokenPoolsBalance.sol
    sed -i 's|IERC20(0)|IERC20(address(0))|g' vault/FlashLoans.sol
    sed -i 's|IERC20(0)|IERC20(address(0))|g' vault/PoolTokens.sol
    sed -i 's|uint256(msg\.sender)|uint256(uint160(msg.sender))|g' pools/BasePool.sol
    sed -i 's|uint256(msg\.sender)|uint256(uint160(msg.sender))|g' pools/weighted/WeightedPool2Tokens.sol
    sed -i 's|address(uint256(_data))|address(uint160(uint256(_data)))|g' lib/openzeppelin/Create2.sol
    sed -i 's|address(uint256(poolId) >> (12 \* 8))|address(uint160(uint256(poolId) >> (12 * 8)))|g' vault/PoolRegistry.sol
    sed -i 's|bytes32(uint256(pool))|bytes32(uint256(uint160(pool)))|g' vault/PoolRegistry.sol
    popd

    # The test suite uses forked mainnet and an expiration period that's too short.
    # TODO: Remove when https://github.com/element-fi/elf-contracts/issues/243 is fixed.
    sed -i 's|^\s*require(_expiration - block\.timestamp < _unitSeconds);\s*$||g' contracts/ConvergentCurvePool.sol

    # Disable tests that won't pass on the ir presets due to Hardhat heuristics. Note that this also disables
    # them for other presets but that's fine - we want same code run for benchmarks to be comparable.
    # TODO: Remove this when Hardhat adjusts heuristics for IR (https://github.com/nomiclabs/hardhat/issues/2115).
    sed -i 's|it(\("fails to withdraw more shares than in balance"\)|it.skip(\1|g' test/compoundAssetProxyTest.ts
    sed -i 's|it(\("should prevent withdrawal of Principal Tokens and Interest Tokens before the tranche expires "\)|it.skip(\1|g' test/trancheTest.ts
    sed -i 's|it(\("should prevent withdrawal of more Principal Tokens and Interest Tokens than the user has"\)|it.skip(\1|g' test/trancheTest.ts

    # This test file is very flaky. There's one particular cases that fails randomly (see
    # https://github.com/element-fi/elf-contracts/issues/240) but some others also depends on an external
    # service which makes tests time out when that service is down.
    # "ProviderError: Too Many Requests error received from eth-mainnet.alchemyapi.io"
    rm test/mockERC20YearnVaultTest.ts

    # Several tests fail unless we use the exact versions hard-coded in package-lock.json
    #neutralize_package_lock

    neutralize_package_json_hooks
    force_hardhat_compiler_binary "$config_file" "$BINARY_TYPE" "$BINARY_PATH"
    force_hardhat_compiler_settings "$config_file" "$(first_word "$SELECTED_PRESETS")" "$config_var"
    force_hardhat_unlimited_contract_size "$config_file" "$config_var"
    npm install

    replace_version_pragmas

    for preset in $SELECTED_PRESETS; do
        hardhat_run_test "$config_file" "$preset" "${compile_only_presets[*]}" compile_fn test_fn "$config_var"
        store_benchmark_report hardhat elementfi "$repo" "$preset"
    done
}

external_test ElementFi elementfi_test
