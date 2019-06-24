#! /bin/bash
#------------------------------------------------------------------------------
# Bash script to execute the Solidity tests by CircleCI.
#
# The documentation for solidity is hosted at:
#
#     https://solidity.readthedocs.org
#
# ------------------------------------------------------------------------------
# Configuration Environment Variables:
#
#     EVM=version_string      Specifies EVM version to compile for (such as homestead, etc)
#     OPTIMIZE=1              Enables backend optimizer
#     ABI_ENCODER_V2=1        Enables ABI encoder version 2
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
# (c) 2016-2019 solidity contributors.
# ------------------------------------------------------------------------------
set -e

OPTIMIZE=${OPTIMIZE:-"0"}
EVM=${EVM:-"invalid"}
WORKDIR=${CIRCLE_WORKING_DIRECTORY:-.}
SOLTEST_IPC=${SOLTEST_IPC:-1}
REPODIR="$(realpath $(dirname $0)/..)"
ALETH_PATH="/usr/bin/aleth"

source "${REPODIR}/scripts/common.sh"
# Test result output directory (CircleCI is reading test results from here)
mkdir -p test_results

ALETH_PID=$(run_aleth)

function cleanup() {
	safe_kill $ALETH_PID $ALETH_PATH
}
trap cleanup INT TERM

# in case we run with ASAN enabled, we must increase stck size.
ulimit -s 16384

BOOST_TEST_ARGS="--color_output=no --show_progress=yes --logger=JUNIT,error,test_results/$EVM.xml"
SOLTEST_ARGS="--evm-version=$EVM --ipcpath "${WORKDIR}/geth.ipc" $flags"
test "${SOLTEST_IPC}" = "1" || SOLTEST_ARGS="$SOLTEST_ARGS --no-ipc"
test "${OPTIMIZE}" = "1" && SOLTEST_ARGS="${SOLTEST_ARGS} --optimize"
test "${ABI_ENCODER_V2}" = "1" && SOLTEST_ARGS="${SOLTEST_ARGS} --abiencoderv2 --optimize-yul"

echo "Running ${REPODIR}/build/test/soltest ${BOOST_TEST_ARGS} -- ${SOLTEST_ARGS}"

${REPODIR}/build/test/soltest ${BOOST_TEST_ARGS} -- ${SOLTEST_ARGS}
