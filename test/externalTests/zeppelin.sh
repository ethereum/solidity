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

verify_input "$@"
BINARY_TYPE="$1"
BINARY_PATH="$2"

function compile_fn { npm run compile; }
function test_fn { npm test; }

function zeppelin_test
{
    local repo="https://github.com/OpenZeppelin/openzeppelin-contracts.git"
    local branch=master
    local config_file="hardhat.config.js"
    local min_optimizer_level=1
    local max_optimizer_level=3

    local selected_optimizer_levels
    selected_optimizer_levels=$(circleci_select_steps "$(seq "$min_optimizer_level" "$max_optimizer_level")")
    print_optimizer_levels_or_exit "$selected_optimizer_levels"

    setup_solc "$DIR" "$BINARY_TYPE" "$BINARY_PATH"
    download_project "$repo" "$branch" "$DIR"

    neutralize_package_json_hooks
    force_hardhat_compiler_binary "$config_file" "$BINARY_TYPE" "$BINARY_PATH"
    force_hardhat_compiler_settings "$config_file" "$min_optimizer_level"
    npm install

    replace_version_pragmas

    for level in $selected_optimizer_levels; do
        hardhat_run_test "$config_file" "$level" compile_fn test_fn
    done
}

external_test Zeppelin zeppelin_test
