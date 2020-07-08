#!/usr/bin/env bash

#------------------------------------------------------------------------------
# Prints version of the Solidity compiler that the source code corresponds to.
#
# The documentation for solidity is hosted at:
#
#     https://solidity.readthedocs.org
#
# ------------------------------------------------------------------------------
# SPDX-License-Identifier: GPL-3.0
#------------------------------------------------------------------------------

set -e

grep -oP "PROJECT_VERSION \"?\K[0-9.]+(?=\")"? $(dirname "$0")/../CMakeLists.txt
