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
function test_fn { yarn test; }

function chainlink_test
{
    local repo="https://github.com/solidity-external-tests/chainlink"
    local ref_type=branch
    local ref=develop_080
    local config_file="hardhat.config.ts"
    local config_var=config

    local compile_only_presets=(
        legacy-no-optimize        # Tests crash on a machine with 8 GB of RAM in CI "FATAL ERROR: Ineffective mark-compacts near heap limit Allocation failed - JavaScript heap out of memory"
    )
    local settings_presets=(
        "${compile_only_presets[@]}"
        #ir-no-optimize           # Compilation fails with "YulException: Variable var__value_775 is 1 slot(s) too deep inside the stack."
        #ir-optimize-evm-only     # Compilation fails with "YulException: Variable var__value_10 is 1 slot(s) too deep inside the stack"
        ir-optimize-evm+yul
        legacy-optimize-evm-only  # NOTE: This requires >= 4 GB RAM in CI not to crash
        legacy-optimize-evm+yul   # NOTE: This requires >= 4 GB RAM in CI not to crash
    )

    [[ $SELECTED_PRESETS != "" ]] || SELECTED_PRESETS=$(circleci_select_steps_multiarg "${settings_presets[@]}")
    print_presets_or_exit "$SELECTED_PRESETS"

    setup_solc "$DIR" "$BINARY_TYPE" "$BINARY_PATH"
    download_project "$repo" "$ref_type" "$ref" "$DIR"

    cd "contracts/"

    # Disable tests that won't pass on the ir presets due to Hardhat heuristics. Note that this also disables
    # them for other presets but that's fine - we want same code run for benchmarks to be comparable.
    # TODO: Remove this when Hardhat adjusts heuristics for IR (https://github.com/nomiclabs/hardhat/issues/2115).
    sed -i "s|\(it\)\(('reverts'\)|\1.skip\2|g" test/v0.6/BasicConsumer.test.ts
    sed -i "s|\(it\)\(('has a reasonable gas cost \[ @skip-coverage \]'\)|\1.skip\2|g" test/v0.6/BasicConsumer.test.ts
    sed -i "s|\(describe\)\(('#add[^']*'\)|\1.skip\2|g" test/v0.6/Chainlink.test.ts
    sed -i "s|\(it\)\(('throws'\)|\1.skip\2|g" test/v0.6/SignedSafeMath.test.ts
    sed -i "s|\(it\)\(('reverts when not enough LINK in the contract'\)|\1.skip\2|g" test/v0.*/VRFD20.test.ts
    sed -i "s|\(it\)\(('errors while parsing invalid cron strings'\)|\1.skip\2|g" test/v0.8/Cron.test.ts
    sed -i "s|\(it\)\(('reverts if the amount passed in data mismatches actual amount sent'\)|\1.skip\2|g" test/v0.8/KeeperRegistrar.test.ts
    sed -i "s|\(it\)\(('reverts if the sender passed in data mismatches actual sender'\)|\1.skip\2|g" test/v0.8/KeeperRegistrar.test.ts
    sed -i "s|\(it\)\(('reverts if the admin address is 0x0000...'\)|\1.skip\2|g" test/v0.8/KeeperRegistrar.test.ts
    sed -i "s|\(it\)\(('reverts if not called with more or less than 32 bytes'\)|\1.skip\2|g" test/v0.8/KeeperRegistry.test.ts
    sed -i "s|\(context\)\(('when permissions are not set'\)|\1.skip\2|g" test/v0.8/KeeperRegistry.test.ts

    # In some cases Hardhat does not detect revert reasons properly via IR.
    # TODO: Remove this when https://github.com/NomicFoundation/hardhat/issues/2453 gets fixed.
    sed -i "s|\(it\)\(('does not allow the specified address to start new rounds'\)|\1.skip\2|g" test/v0.6/FluxAggregator.test.ts
    sed -i "s|\(describe\)\(('when called by a stranger'\)|\1.skip\2|g" test/v0.6/FluxAggregator.test.ts
    sed -i "s|\(describe\)\(('if the access control is turned on'\)|\1.skip\2|g" test/v0.*/Flags.test.ts
    sed -i "s|\(it\)\(('respects the access controls of #getFlag'\)|\1.skip\2|g" test/v0.*/Flags.test.ts
    sed -i "s|\(describe\)\(('setting 0 authorized senders'\)|\1.skip\2|g" test/v0.7/AuthorizedForwarder.test.ts
    sed -i "s|\(it\)\(('cannot add an authorized node'\)|\1.skip\2|g" test/v0.7/AuthorizedForwarder.test.ts
    sed -i "s|\(it\)\(('should disallow reads on AggregatorV2V3Interface functions when consuming contract is not whitelisted'\)|\1.skip\2|g" test/v0.8/dev/ArbitrumSequencerUptimeFeed.test.ts
    sed -i "s|\(it\)\(('should not be callable by non-owners'\)|\1.skip\2|g" test/v0.8/dev/CrossDomainOwnable.test.ts
    sed -i "s|\(it\)\(('should not be callable by non pending-owners'\)|\1.skip\2|g" test/v0.8/dev/CrossDomainOwnable.test.ts
    sed -i "s|\(it\)\(('cannot add a consumer to a nonexistent subscription'\)|\1.skip\2|g" test/v0.8/dev/VRFCoordinatorV2Mock.test.ts
    sed -i "s|\(it\)\(('cannot remove a consumer from a nonexistent subscription'\)|\1.skip\2|g" test/v0.8/dev/VRFCoordinatorV2Mock.test.ts
    sed -i "s|\(it\)\(('cannot remove a consumer after it is already removed'\)|\1.skip\2|g" test/v0.8/dev/VRFCoordinatorV2Mock.test.ts
    sed -i "s|\(it\)\(('fails to fulfill without being a valid consumer'\)|\1.skip\2|g" test/v0.8/dev/VRFCoordinatorV2Mock.test.ts

    # Disable tests with hard-coded gas expectations.
    sed -i "s|\(it\)\(('not use too much gas \[ @skip-coverage \]'\)|\1.skip\2|g" test/v0.6/FluxAggregator.test.ts
    sed -i "s|\(it\)\(('has a large enough gas overhead to cover upkeeps that use all their gas \[ @skip-coverage \]'\)|\1.skip\2|g" test/v0.*/KeeperRegistry*.test.ts
    sed -i "s|\(it\)\(('only pays .\+'\)|\1.skip\2|g" test/v0.*/KeeperRegistry*.test.ts
    sed -i "s|\(it\)\(('uses a specific amount of gas \[ @skip-coverage \]'\)|\1.skip\2|g" test/v0.8/ValidatorProxy.test.ts
    sed -i "s|\(describe\)\(('Gas costs'\)|\1.skip\2|g" test/v0.8/dev/ArbitrumSequencerUptimeFeed.test.ts

    neutralize_package_lock
    neutralize_package_json_hooks
    name_hardhat_default_export "$config_file" "$config_var"
    force_hardhat_compiler_binary "$config_file" "$BINARY_TYPE" "$BINARY_PATH"
    force_hardhat_compiler_settings "$config_file" "$(first_word "$SELECTED_PRESETS")" "$config_var"
    force_hardhat_unlimited_contract_size "$config_file" "$config_var"
    yarn install
    yarn add hardhat-gas-reporter

    replace_version_pragmas

    for preset in $SELECTED_PRESETS; do
        hardhat_run_test "$config_file" "$preset" "${compile_only_presets[*]}" compile_fn test_fn "$config_var"
        store_benchmark_report hardhat chainlink "$repo" "$preset"
    done
}

external_test Chainlink chainlink_test
