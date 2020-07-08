#!/usr/bin/env bash

#------------------------------------------------------------------------------
# Bash script which calls the Emscripten "publish binary" script on Ubuntu
# and does nothing on macOS.  We should probably merge these two scripts in
# the near future.
#
# The documentation for solidity is hosted at:
#
#     https://solidity.readthedocs.org
#
# ------------------------------------------------------------------------------
# SPDX-License-Identifier: GPL-3.0
#------------------------------------------------------------------------------

if [[ "$OSTYPE" != "darwin"* ]]; then
    ./scripts/travis-emscripten/publish_binary.sh
fi
