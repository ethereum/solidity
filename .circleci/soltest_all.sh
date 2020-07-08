#! /bin/bash
#------------------------------------------------------------------------------
# Bash script to execute the Solidity tests by CircleCI.
#
# The documentation for solidity is hosted at:
#
#     https://solidity.readthedocs.org
#
# ------------------------------------------------------------------------------
# SPDX-License-Identifier: GPL-3.0
# ------------------------------------------------------------------------------
set -e

REPODIR="$(realpath $(dirname $0)/..)"

for OPTIMIZE in 0 1; do
    for EVM in homestead byzantium constantinople petersburg istanbul; do
        EVM=$EVM OPTIMIZE=$OPTIMIZE ${REPODIR}/.circleci/soltest.sh
    done
done

EVM=istanbul OPTIMIZE=1 ABI_ENCODER_V2=1 ${REPODIR}/.circleci/soltest.sh
