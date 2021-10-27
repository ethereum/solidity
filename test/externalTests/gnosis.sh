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

verify_input "$1"
SOLJSON="$1"

function compile_fn { npx truffle compile; }
function test_fn { npm test; }

function gnosis_safe_test
{
    local repo="https://github.com/solidity-external-tests/safe-contracts.git"
    local branch=development_080
    local config_file="truffle-config.js"
    local min_optimizer_level=2
    local max_optimizer_level=3

    setup_solcjs "$DIR" "$SOLJSON"
    download_project "$repo" "$branch" "$DIR"

    sed -i 's|github:gnosis/mock-contract#sol_0_5_0|github:solidity-external-tests/mock-contract#master_080|g' package.json

    neutralize_package_lock
    neutralize_package_json_hooks
    force_truffle_compiler_settings "$config_file" "${DIR}/solc" "$min_optimizer_level"
    npm install --package-lock

    replace_version_pragmas
    force_solc_modules "${DIR}/solc"

    for level in $(seq "$min_optimizer_level" "$max_optimizer_level"); do
        truffle_run_test "$config_file" "${DIR}/solc" "$level" compile_fn test_fn
    done
}

external_test Gnosis-Safe gnosis_safe_test
