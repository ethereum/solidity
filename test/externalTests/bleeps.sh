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

function compile_fn { npm run compile; }
# NOTE: `npm run test` runs `mocha` which seems to disable the gas reporter.
function test_fn { HARDHAT_DEPLOY_FIXTURE=true npx --no hardhat --no-compile test; }

function bleeps_test
{
    local repo="https://github.com/wighawag/bleeps"
    local ref_type=branch
    local ref=main
    local config_file="hardhat.config.ts"
    local config_var=config

    local compile_only_presets=()
    local settings_presets=(
        "${compile_only_presets[@]}"
        #ir-no-optimize            # Compilation fails with: "YulException: Variable param_0 is 2 slot(s) too deep inside the stack."
        #ir-optimize-evm-only      # Compilation fails with: "YulException: Variable param_0 is 2 slot(s) too deep inside the stack."
        ir-optimize-evm+yul
        #legacy-no-optimize        # Compilation fails with: "CompilerError: Stack too deep, try removing local variables."
        #legacy-optimize-evm-only  # Compilation fails with: "CompilerError: Stack too deep, try removing local variables."
        legacy-optimize-evm+yul
    )

    [[ $SELECTED_PRESETS != "" ]] || SELECTED_PRESETS=$(circleci_select_steps_multiarg "${settings_presets[@]}")
    print_presets_or_exit "$SELECTED_PRESETS"

    setup_solc "$DIR" "$BINARY_TYPE" "$BINARY_PATH"
    download_project "$repo" "$ref_type" "$ref" "$DIR"

    pushd "common-lib/"
    neutralize_package_json_hooks
    npm install
    npm run build
    popd

    pushd "contracts/"
    sed -i 's|"bleeps-common": "workspace:\*",|"bleeps-common": "file:../common-lib/",|g' package.json

    sed -i 's/function() public/fallback() external/g' src/externals/WETH9.sol
    sed -i 's/this\.balance/address(this).balance/g' src/externals/WETH9.sol
    sed -i 's/uint(-1)/type(uint).max/g' src/externals/WETH9.sol
    sed -i 's/msg\.sender\.transfer(/payable(msg.sender).transfer(/g' src/externals/WETH9.sol
    sed -i 's/^\s*\(Deposit\|Withdrawal\|Approval\|Transfer\)(/emit \1(/g' src/externals/WETH9.sol

    # This test does not currently pass due to an upstream problem.
    # TODO: Remove this line when https://github.com/wighawag/bleeps/issues/2 is fixed
    rm test/BleepsDAO.governor.test.ts

    neutralize_package_lock
    neutralize_package_json_hooks
    force_hardhat_compiler_binary "$config_file" "$BINARY_TYPE" "$BINARY_PATH"
    force_hardhat_compiler_settings "$config_file" "$(first_word "$SELECTED_PRESETS")" "$config_var"
    npm install npm-run-all
    npm install

    # TODO: Bleeps depends on OpenZeppelin 4.3.2, which is affected by
    # https://github.com/OpenZeppelin/openzeppelin-contracts/pull/3293.
    # Forcing OZ >= 4.6.0 fixes this but it also causes a lot of unrelated compilation errors.
    # Remove this when Bleeps gets updated to support newer OpenZeppelin.
    perl -i -0pe \
        "s/(function hashProposal\(\n        address\[\] )calldata( targets,\n        uint256\[\] )calldata( values,\n        bytes\[\] )calldata( calldatas,)/\1memory\2memory\3memory\4/g" \
        node_modules/@openzeppelin/contracts/governance/IGovernor.sol

    replace_version_pragmas

    for preset in $SELECTED_PRESETS; do
        hardhat_run_test "$config_file" "$preset" "${compile_only_presets[*]}" compile_fn test_fn "$config_var"
        store_benchmark_report hardhat bleeps "$repo" "$preset"
    done

    popd
}

external_test Bleeps bleeps_test
