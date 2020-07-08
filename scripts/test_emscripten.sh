#!/usr/bin/env bash

#------------------------------------------------------------------------------
# Bash script to execute the Solidity tests using the emscripten binary.
#
# The documentation for solidity is hosted at:
#
#     https://solidity.readthedocs.org
#
# ------------------------------------------------------------------------------
# SPDX-License-Identifier: GPL-3.0
#------------------------------------------------------------------------------

set -e

if test -z "$1"; then
	BUILD_DIR="emscripten_build"
else
	BUILD_DIR="$1"
fi

REPO_ROOT=$(cd $(dirname "$0")/.. && pwd)
SOLJSON="$REPO_ROOT/$BUILD_DIR/libsolc/soljson.js"
VERSION=$("$REPO_ROOT"/scripts/get_version.sh)

echo "Running solcjs tests...."
"$REPO_ROOT/test/externalTests/solc-js/solc-js.sh" "$SOLJSON" "$VERSION"
