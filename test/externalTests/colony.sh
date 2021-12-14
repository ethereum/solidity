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

function compile_fn { yarn run provision:token:contracts; }
function test_fn { yarn run test:contracts; }

function colony_test
{
    local repo="https://github.com/solidity-external-tests/colonyNetwork.git"
    local branch=develop_080
    local config_file="truffle.js"
    # On levels 1 and 2 it compiles but tests run out of gas
    local min_optimizer_level=3
    local max_optimizer_level=3

    local selected_optimizer_levels
    selected_optimizer_levels=$(circleci_select_steps "$(seq "$min_optimizer_level" "$max_optimizer_level")")
    print_optimizer_levels_or_exit "$selected_optimizer_levels"

    setup_solc "$DIR" "$BINARY_TYPE" "$BINARY_PATH"
    download_project "$repo" "$branch" "$DIR"
    [[ $BINARY_TYPE == native ]] && replace_global_solc "$BINARY_PATH"

    neutralize_package_json_hooks
    force_truffle_compiler_settings "$config_file" "$BINARY_TYPE" "${DIR}/solc" "$min_optimizer_level"
    yarn install
    git submodule update --init

    cd lib
    rm -Rf dappsys
    git clone https://github.com/solidity-external-tests/dappsys-monolithic.git -b master_080 dappsys
    cd ..

    replace_version_pragmas
    [[ $BINARY_TYPE == solcjs ]] && force_solc_modules "${DIR}/solc"

    for level in $selected_optimizer_levels; do
        truffle_run_test "$config_file" "$BINARY_TYPE" "${DIR}/solc" "$level" compile_fn test_fn
    done
}

external_test ColonyNetworks colony_test
