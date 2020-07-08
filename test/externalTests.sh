#!/usr/bin/env bash

#------------------------------------------------------------------------------
# Bash script to run external Solidity tests.
#
# Argument: Path to soljson.js to test.
#
# Requires npm, networking access and git to download the tests.
#
# ------------------------------------------------------------------------------
# SPDX-License-Identifier: GPL-3.0
#------------------------------------------------------------------------------

set -e

if [ ! -f "$1" ]
then
  echo "Usage: $0 <path to soljson.js>"
  exit 1
fi

SOLJSON="$1"
REPO_ROOT="$(dirname "$0")"

source scripts/common.sh
source test/externalTests/common.sh

printTask "Running external tests..."

$REPO_ROOT/externalTests/zeppelin.sh "$SOLJSON"
$REPO_ROOT/externalTests/gnosis.sh "$SOLJSON"
$REPO_ROOT/externalTests/colony.sh "$SOLJSON"

# Disabled temporarily as it needs to be updated to latest Truffle first.
#test_truffle Gnosis https://github.com/axic/pm-contracts.git solidity-050
