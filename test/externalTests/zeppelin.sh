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

# Disable shellcheck errors on quoted special chars like backticks. Too many false-positives.
# shellcheck disable=SC2016

set -e
# Temporary(?) fix to up the heap limit for node in order to prevent 'out of heap errors'
export NODE_OPTIONS="--max-old-space-size=4096"

source scripts/common.sh
source scripts/externalTests/common.sh

REPO_ROOT=$(realpath "$(dirname "$0")/../..")

verify_input "$@"
BINARY_TYPE="$1"
BINARY_PATH="$(realpath "$2")"
SELECTED_PRESETS="$3"

function compile_fn { npm run compile; }
function test_fn { npm test; }

function zeppelin_test
{
    local repo="https://github.com/OpenZeppelin/openzeppelin-contracts.git"
    local ref_type=branch
    local ref="master"
    local config_file="hardhat.config.js"

    local compile_only_presets=()
    local settings_presets=(
        "${compile_only_presets[@]}"
        #ir-no-optimize           # Compilation fails with "YulException: Variable var_account_852 is 4 slot(s) too deep inside the stack."
        #ir-optimize-evm-only     # Compilation fails with "YulException: Variable var_account_852 is 4 slot(s) too deep inside the stack."
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
    # TODO: Remove the lines below when Hardhat adjusts heuristics for IR (https://github.com/nomiclabs/hardhat/issues/3750).
    sed -i "s|it(\('proxy admin cannot call delegated functions',\)|it.skip(\1|g" test/proxy/transparent/TransparentUpgradeableProxy.behaviour.js
    sed -i "s|describe(\('when the given implementation is the zero address'\)|describe.skip(\1|g" test/proxy/transparent/TransparentUpgradeableProxy.behaviour.js
    sed -i "s|describe(\('when the new proposed admin is the zero address'\)|describe.skip(\1|g" test/proxy/transparent/TransparentUpgradeableProxy.behaviour.js
    # In some cases Hardhat does not detect revert reasons properly via IR.
    sed -i "s|it(\('reverts if the current value is 0'\)|it.skip(\1|g" test/utils/Counters.test.js
    sed -i "s|it(\('prevent unauthorized maintenance'\)|it.skip(\1|g" test/governance/TimelockController.test.js
    sed -i "s|it(\('cannot cancel invalid operation'\)|it.skip(\1|g" test/governance/TimelockController.test.js
    sed -i "s|it(\('cannot call onlyInitializable function outside the scope of an initializable function'\)|it.skip(\1|g" test/proxy/utils/Initializable.test.js
    sed -i "s|it(\('other accounts cannot unpause'\)|it.skip(\1|g" test/token/ERC20/presets/ERC20PresetMinterPauser.test.js
    sed -i "s|it(\('other accounts cannot unpause'\)|it.skip(\1|g" test/token/ERC1155/presets/ERC1155PresetMinterPauser.test.js
    sed -i "s|it(\('other accounts cannot pause'\)|it.skip(\1|g" test/token/ERC20/presets/ERC20PresetMinterPauser.test.js
    sed -i "s|it(\('other accounts cannot pause'\)|it.skip(\1|g" test/token/ERC1155/presets/ERC1155PresetMinterPauser.test.js
    sed -i "s|it(\('cannot be released before time limit'\)|it.skip(\1|g" test/token/ERC20/utils/TokenTimelock.test.js
    sed -i "s|it(\('cannot be released just before time limit'\)|it.skip(\1|g" test/token/ERC20/utils/TokenTimelock.test.js
    sed -i "s|it(\('reverts when sending non-zero amounts'\)|it.skip(\1|g" test/utils/Address.test.js
    sed -i "s|it(\('reverts when sending more than the balance'\)|it.skip(\1|g" test/utils/Address.test.js
    sed -i "s|it(\('fails deploying a contract if the bytecode length is zero'\)|it.skip(\1|g" test/utils/Create2.test.js
    sed -i "s|it(\('fails deploying a contract if factory contract does not have sufficient balance'\)|it.skip(\1|g" test/utils/Create2.test.js
    sed -i "s|it(\('reverts on withdrawals'\)|it.skip(\1|g" test/utils/escrow/ConditionalEscrow.test.js
    sed -i "s|it(\('does not allow beneficiary withdrawal'\)|it.skip(\1|g" test/utils/escrow/RefundEscrow.test.js
    sed -i "s|it(\('rejects deposits'\)|it.skip(\1|g" test/utils/escrow/RefundEscrow.test.js
    sed -i "s|it(\('does not allow 0xffffffff'\)|it.skip(\1|g" test/utils/introspection/ERC165Storage.test.js
    sed -i "s|it(\('reverts when casting -1'\)|it.skip(\1|g" test/utils/math/SafeCast.test.js
    sed -i 's|it(\(`reverts when casting [^`]\+`\)|it.skip(\1|g' test/utils/math/SafeCast.test.js
    sed -i "s|it(\('reverts if index is greater than supply'\)|it.skip(\1|g" test/token/ERC721/ERC721.behavior.js
    sed -i "s|it(\('burns all tokens'\)|it.skip(\1|g" test/token/ERC721/ERC721.behavior.js
    sed -i "s|it(\('other accounts cannot pause'\)|it.skip(\1|g" test/token/ERC721/presets/ERC721PresetMinterPauserAutoId.test.js
    sed -i "s|it(\('other accounts cannot unpause'\)|it.skip(\1|g" test/token/ERC721/presets/ERC721PresetMinterPauserAutoId.test.js
    sed -i "s|it(\('guards transfer against invalid user'\)|it.skip(\1|g" test/access/Ownable2Step.test.js
    sed -i "s|it(\('reverting initialization'\)|it.skip(\1|g" test/proxy/beacon/BeaconProxy.test.js
    sed -i "s|describe(\('reverting initialization'\)|describe.skip(\1|g" test/proxy/Proxy.behaviour.js
    sed -i "s|it(\('does not allow remote callback'\)|it.skip(\1|g" test/security/ReentrancyGuard.test.js
    sed -i "s|shouldBehaveLikeAccessControlDefaultAdminRules|\/\/&|" test/access/AccessControlDefaultAdminRules.test.js

    # Here only the testToInt(248) and testToInt(256) cases fail so change the loop range to skip them
    sed -i "s|range(8, 256, 8)\(.forEach(bits => testToInt(bits));\)|range(8, 240, 8)\1|" test/utils/math/SafeCast.test.js

    neutralize_package_lock
    neutralize_package_json_hooks
    force_hardhat_compiler_binary "$config_file" "$BINARY_TYPE" "$BINARY_PATH"
    force_hardhat_compiler_settings "$config_file" "$(first_word "$SELECTED_PRESETS")"
    npm install

    replace_version_pragmas

    for preset in $SELECTED_PRESETS; do
        hardhat_run_test "$config_file" "$preset" "${compile_only_presets[*]}" compile_fn test_fn
        store_benchmark_report hardhat zeppelin "$repo" "$preset"
    done
}

external_test Zeppelin zeppelin_test
