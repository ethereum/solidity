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

if [ ! -f "$1" ]
then
  echo "Usage: $0 <path to soljson.js>"
  exit 1
fi

SOLJSON="$1"
REPO_ROOT="$(dirname "$0")"
TEST_DIR=""$REPO_ROOT"/test/externalTests"
TMP_LOG_DIR=$(mktemp -d)

source scripts/common.sh
source test/externalTests/common.sh

printTask "Running external tests..."
# Unset so that all tests are forced to complete
unset -e
for test in zeppelin gnosis colony ens;
do
  $TEST_DIR/$test.sh "$SOLJSON" &> $TMP_LOG_DIR/$test.log &
done

printTask "Waiting for external tests to complete..."
wait

set -e
printTask "Logging test results..."
rm -f $TEST_DIR/externalTests.log
cat $TMP_LOG_DIR/*.log >> $TEST_DIR/externalTests.log
# Remove temp log dir
rm -rf $TMP_LOG_DIR

# Disabled temporarily as it needs to be updated to latest Truffle first.
#test_truffle Gnosis https://github.com/axic/pm-contracts.git solidity-050
