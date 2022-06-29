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

function compile_fn { npm run build; }
function test_fn { npm test; }

function gnosis_safe_test
{
    local repo="https://github.com/safe-global/safe-contracts.git"
    local ref_type=branch
    local ref=main
    local config_file="hardhat.config.ts"
    local config_var=userConfig

    local compile_only_presets=()
    local settings_presets=(
        "${compile_only_presets[@]}"
        #ir-no-optimize            # Compilation fails with "YulException: Variable var_call_430_mpos is 1 slot(s) too deep inside the stack."
        #ir-optimize-evm-only      # Compilation fails with "YulException: Variable var_call_430_mpos is 1 slot(s) too deep inside the stack."
        ir-optimize-evm+yul
        legacy-no-optimize
        legacy-optimize-evm-only
        legacy-optimize-evm+yul
    )

    [[ $SELECTED_PRESETS != "" ]] || SELECTED_PRESETS=$(circleci_select_steps_multiarg "${settings_presets[@]}")
    print_presets_or_exit "$SELECTED_PRESETS"

    setup_solc "$DIR" "$BINARY_TYPE" "$BINARY_PATH"
    download_project "$repo" "$ref_type" "$ref" "$DIR"
    [[ $BINARY_TYPE == native ]] && replace_global_solc "$BINARY_PATH"

    # NOTE: The patterns below intentionally have hard-coded versions.
    # When the upstream updates them, there's a chance we can just remove the regex.
    sed -i 's|"@gnosis\.pm/mock-contract": "\^4\.0\.0"|"@gnosis.pm/mock-contract": "github:solidity-external-tests/mock-contract#master_080"|g' package.json
    sed -i 's|"@openzeppelin/contracts": "\^3\.4\.0"|"@openzeppelin/contracts": "^4.0.0"|g' package.json

    # Disable two tests failing due to Hardhat's heuristics not yet updated to handle solc 0.8.10.
    # TODO: Remove this when Hardhat implements them (https://github.com/nomiclabs/hardhat/issues/2051).
    sed -i "s|\(it\)\(('should revert if called directly', async () => {\)|\1.skip\2|g" test/handlers/CompatibilityFallbackHandler.spec.ts

    # Disable tests that won't pass on the ir presets due to Hardhat heuristics. Note that this also disables
    # them for other presets but that's fine - we want same code run for benchmarks to be comparable.
    # TODO: Remove this when Hardhat adjusts heuristics for IR (https://github.com/nomiclabs/hardhat/issues/2115).
    sed -i "s|\(it\)\(('should not allow to call setup on singleton'\)|\1.skip\2|g" test/core/GnosisSafe.Setup.spec.ts
    # TODO: Remove this when https://github.com/NomicFoundation/hardhat/issues/2453 gets fixed.
    sed -i 's|\(it\)\(("changes the expected storage slot without touching the most important ones"\)|\1.skip\2|g' test/libraries/SignMessageLib.spec.ts
    sed -i "s|\(it\)\(('can be used only via DELEGATECALL opcode'\)|\1.skip\2|g" test/libraries/SignMessageLib.spec.ts
    sed -i 's|\(describe\)\(("Upgrade from Safe 1.1.1"\)|\1.skip\2|g' test/migration/UpgradeFromSafe111.spec.ts
    sed -i 's|\(describe\)\(("Upgrade from Safe 1.2.0"\)|\1.skip\2|g' test/migration/UpgradeFromSafe120.spec.ts

    # TODO: Remove this when Gnosis merges https://github.com/gnosis/safe-contracts/pull/394
    sed -i "s|\(function isValidSignature(bytes \)calldata\( _data, bytes \)calldata\( _signature)\)|\1memory\2memory\3|g" contracts/handler/CompatibilityFallbackHandler.sol

    # TODO: Remove this when https://github.com/NomicFoundation/hardhat/issues/2453 gets fixed.
    sed -i "s|it\(('should enforce delegatecall'\)|it.skip\1|g" test/accessors/SimulateTxAccessor.spec.ts
    sed -i "s|it\(('can only be called from Safe itself'\)|it.skip\1|g" test/libraries/Migration.spec.ts
    sed -i "s|it\(('should enforce delegatecall to MultiSend'\)|it.skip\1|g" test/libraries/MultiSend.spec.ts

    neutralize_package_lock
    neutralize_package_json_hooks
    force_hardhat_compiler_binary "$config_file" "$BINARY_TYPE" "$BINARY_PATH"
    force_hardhat_compiler_settings "$config_file" "$(first_word "$SELECTED_PRESETS")" "$config_var"
    npm install
    npm install hardhat-gas-reporter

    # Typescript compilation fails with typescript >= 4.7:
    # Error: Debug Failure. False expression: Non-string value passed to `ts.resolveTypeReferenceDirective`
    npm install "typescript@<4.7.0"

    # With ethers.js 5.6.2 many tests for revert messages fail.
    # TODO: Remove when https://github.com/ethers-io/ethers.js/discussions/2849 is resolved.
    npm install ethers@5.6.1

    # Note that ethers@5.6.1 depends on @ethersproject/contracts@5.6.0 while the dependency on hardhat-deploy
    # pulls @ethersproject/contracts@5.6.1 (latest). Force 5.6.0 to avoid errors due to having two copies.
    npm install @ethersproject/contracts@5.6.0

    # Hardhat 2.9.5 introduced a bug with handling padded arguments to getStorageAt().
    # TODO: Remove when https://github.com/NomicFoundation/hardhat/issues/2709 is fixed.
    npm install hardhat@2.9.4

    replace_version_pragmas
    [[ $BINARY_TYPE == solcjs ]] && force_solc_modules "${DIR}/solc/dist"

    for preset in $SELECTED_PRESETS; do
        hardhat_run_test "$config_file" "$preset" "${compile_only_presets[*]}" compile_fn test_fn "$config_var"
        store_benchmark_report hardhat gnosis "$repo" "$preset"
    done
}

external_test Gnosis-Safe gnosis_safe_test
