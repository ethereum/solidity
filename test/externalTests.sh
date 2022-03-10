#!/usr/bin/env bash

#------------------------------------------------------------------------------
# Bash script to run external Solidity tests.
#
# Argument: Path to soljson.js to test.
#
# Requires npm, networking access and git to download the tests.
#
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
# (c) 2016 solidity contributors.
#------------------------------------------------------------------------------

set -e

source scripts/common.sh
source test/externalTests/common.sh

REPO_ROOT=$(realpath "$(dirname "$0")/..")

verify_input "$@"

printTask "Running external tests..."

"{$REPO_ROOT}/test/externalTests/zeppelin.sh" "$@"
"{$REPO_ROOT}/test/externalTests/gnosis.sh" "$@"
"{$REPO_ROOT}/test/externalTests/colony.sh" "$@"
"{$REPO_ROOT}/test/externalTests/ens.sh" "$@"
"{$REPO_ROOT}/test/externalTests/trident.sh" "$@"
"{$REPO_ROOT}/test/externalTests/euler.sh" "$@"
"{$REPO_ROOT}/test/externalTests/yield-liquidator.sh" "$@"
"{$REPO_ROOT}/test/externalTests/bleeps.sh" "$@"
"{$REPO_ROOT}/test/externalTests/pool-together.sh" "$@"
"{$REPO_ROOT}/test/externalTests/perpetual-pools.sh" "$@"
"{$REPO_ROOT}/test/externalTests/uniswap.sh" "$@"
"{$REPO_ROOT}/test/externalTests/prb-math.sh" "$@"
"{$REPO_ROOT}/test/externalTests/elementfi.sh" "$@"
