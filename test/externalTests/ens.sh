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
source scripts/externalTests/common.sh

REPO_ROOT=$(realpath "$(dirname "$0")/../..")

verify_input "$@"
BINARY_TYPE="$1"
BINARY_PATH="$(realpath "$2")"
SELECTED_PRESETS="$3"

function compile_fn { yarn build; }
function test_fn { yarn test; }

function ens_test
{
    local repo="https://github.com/ensdomains/ens-contracts.git"
    local ref_type=commit
    local ref="083d29a2c50cd0a8307386abf8fadc217b256256"
    local config_file="hardhat.config.js"

    local compile_only_presets=(
        legacy-no-optimize        # Compiles but tests fail to deploy GovernorCompatibilityBravo (code too large).
    )
    local settings_presets=(
        "${compile_only_presets[@]}"
        #ir-no-optimize           # Compilation fails with "YulException: Variable var__945 is 1 slot(s) too deep inside the stack."
        #ir-optimize-evm-only     # Compilation fails with "YulException: Variable var__945 is 1 slot(s) too deep inside the stack."
        ir-optimize-evm+yul      # Needs memory-safe inline assembly patch
        legacy-optimize-evm-only
        legacy-optimize-evm+yul
    )

    [[ $SELECTED_PRESETS != "" ]] || SELECTED_PRESETS=$(circleci_select_steps_multiarg "${settings_presets[@]}")
    print_presets_or_exit "$SELECTED_PRESETS"

    setup_solc "$DIR" "$BINARY_TYPE" "$BINARY_PATH"
    download_project "$repo" "$ref_type" "$ref" "$DIR"

    neutralize_package_lock
    neutralize_package_json_hooks
    force_hardhat_compiler_binary "$config_file" "$BINARY_TYPE" "$BINARY_PATH"
    force_hardhat_compiler_settings "$config_file" "$(first_word "$SELECTED_PRESETS")"
    yarn install

    replace_version_pragmas
    neutralize_packaged_contracts

    # In some cases Hardhat does not detect revert reasons properly via IR.
    # TODO: Remove this when https://github.com/NomicFoundation/hardhat/issues/3365 gets fixed.
    sed -i "s|it\(('Does not allow wrapping a name you do not own',\)|it.skip\1|g" test/wrapper/NameWrapper.js
    sed -i "s|it\(('can set fuses and then burn ability to burn fuses',\)|it.skip\1|g" test/wrapper/NameWrapper.js
    sed -i "s|it\(('can set fuses and burn canSetResolver and canSetTTL',\)|it.skip\1|g" test/wrapper/NameWrapper.js
    sed -i "s|it\(('Cannot be called if CANNOT_TRANSFER is burned\.',\)|it.skip\1|g" test/wrapper/NameWrapper.js
    sed -i "s|it\(('Cannot be called if CANNOT_SET_RESOLVER is burned\.\?',\)|it.skip\1|g" test/wrapper/NameWrapper.js
    sed -i "s|it\(('Cannot be called if CANNOT_SET_TTL is burned\.\?',\)|it.skip\1|g" test/wrapper/NameWrapper.js
    sed -i "s|it\(('Cannot be called if CREATE_SUBDOMAIN is burned and is a new subdomain',\)|it.skip\1|g" test/wrapper/NameWrapper.js
    sed -i "s|it\(('Cannot be called if REPLACE_SUBDOMAIN is burned and is an existing subdomain',\)|it.skip\1|g" test/wrapper/NameWrapper.js
    sed -i "s|it\(('Cannot be called if CANNOT_CREATE_SUBDOMAIN is burned and is a new subdomain',\)|it.skip\1|g" test/wrapper/NameWrapper.js
    sed -i "s|it\(('Cannot be called if PARENT_CANNOT_CONTROL is burned and is an existing subdomain',\)|it.skip\1|g" test/wrapper/NameWrapper.js

    find . -name "*.sol" -exec sed -i -e 's/^\(\s*\)\(assembly\)/\1\/\/\/ @solidity memory-safe-assembly\n\1\2/' '{}' \;

    for preset in $SELECTED_PRESETS; do
        hardhat_run_test "$config_file" "$preset" "${compile_only_presets[*]}" compile_fn test_fn
        store_benchmark_report hardhat ens "$repo" "$preset"
    done
}

external_test ENS ens_test
