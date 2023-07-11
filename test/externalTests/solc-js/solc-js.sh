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

set -e

source scripts/common.sh
source scripts/externalTests/common.sh

SOLJSON="$1"
VERSION="$2"
SOLCJS_CHECKOUT="$3" # optional

[[ $SOLJSON != "" && -f "$SOLJSON" && $VERSION != "" ]] || fail "Usage: $0 <path to soljson.js> <version> [<path to solc-js>]"

function solcjs_test
{
    TEST_DIR=$(pwd)
    SOLCJS_INPUT_DIR="$TEST_DIR"/test/externalTests/solc-js

    # set up solc-js on the branch specified
    setup_solc "$DIR" solcjs "$SOLJSON" master solc/ "$SOLCJS_CHECKOUT"
    cd solc/

    printLog "Updating index.js file..."
    echo "require('./determinism.js');" >> test/index.js

    printLog "Copying determinism.js..."
    cp -f "$SOLCJS_INPUT_DIR/determinism.js" test/

    printLog "Copying contracts..."
    cp -Rf "$SOLCJS_INPUT_DIR/DAO" test/

    printLog "Copying SMTChecker tests..."
    # We do not copy all tests because that takes too long.
    cp -Rf "$TEST_DIR"/test/libsolidity/smtCheckerTests/external_calls test/smtCheckerTests/
    cp -Rf "$TEST_DIR"/test/libsolidity/smtCheckerTests/loops test/smtCheckerTests/
    cp -Rf "$TEST_DIR"/test/libsolidity/smtCheckerTests/invariants test/smtCheckerTests/

    # Update version (needed for some tests)
    echo "Updating package.json to version $VERSION"
    npm version --allow-same-version --no-git-tag-version "$VERSION"

    replace_version_pragmas

    printLog "Running test function..."
    npm test
}

external_test solc-js solcjs_test
