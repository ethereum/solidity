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
# shellcheck disable=SC1091
source scripts/common.sh
# shellcheck disable=SC1091
source test/externalTests/common.sh

verify_input "$1"
export SOLJSON="$1"

function install_fn { npm install; }
function compile_fn { npx truffle compile; }
function test_fn { npm run test; }

function ens_test
{
    export OPTIMIZER_LEVEL=1
    export CONFIG="truffle-config.js"

    truffle_setup https://github.com/solidity-external-tests/ens.git upgrade-0.8.0

    # Use latest Truffle. Older versions crash on the output from 0.8.0.
    force_truffle_version ^5.1.55

    run_install install_fn

    truffle_run_test compile_fn test_fn
}

external_test Ens ens_test
