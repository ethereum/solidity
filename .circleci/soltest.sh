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
#     SOLTEST_FLAGS=<flags>   Appends <flags> to default SOLTEST_ARGS
#
# ------------------------------------------------------------------------------
# SPDX-License-Identifier: GPL-3.0
# ------------------------------------------------------------------------------
set -e

OPTIMIZE=${OPTIMIZE:-"0"}
EVM=${EVM:-"invalid"}
WORKDIR=${CIRCLE_WORKING_DIRECTORY:-.}
REPODIR="$(realpath $(dirname $0)/..)"

source "${REPODIR}/scripts/common.sh"
# Test result output directory (CircleCI is reading test results from here)
mkdir -p test_results

# in case we run with ASAN enabled, we must increase stck size.
ulimit -s 16384

get_logfile_basename() {
    local filename="${EVM}"
    test "${OPTIMIZE}" = "1" && filename="${filename}_opt"
    test "${ABI_ENCODER_V2}" = "1" && filename="${filename}_abiv2"

    echo -ne "${filename}"
}

BOOST_TEST_ARGS="--color_output=no --show_progress=yes --logger=JUNIT,error,test_results/`get_logfile_basename`.xml"
SOLTEST_ARGS="--evm-version=$EVM $SOLTEST_FLAGS"
test "${OPTIMIZE}" = "1" && SOLTEST_ARGS="${SOLTEST_ARGS} --optimize"
test "${ABI_ENCODER_V2}" = "1" && SOLTEST_ARGS="${SOLTEST_ARGS} --abiencoderv2"

echo "Running ${REPODIR}/build/test/soltest ${BOOST_TEST_ARGS} -- ${SOLTEST_ARGS}"

${REPODIR}/build/test/soltest ${BOOST_TEST_ARGS} -- ${SOLTEST_ARGS}
