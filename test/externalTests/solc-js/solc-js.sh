#!/usr/bin/env bash

# ------------------------------------------------------------------------------
# SPDX-License-Identifier: GPL-3.0
#------------------------------------------------------------------------------
source scripts/common.sh
source test/externalTests/common.sh

verify_version_input "$1" "$2"
SOLJSON="$1"
VERSION="$2"

function install_fn { echo "Nothing to install."; }
function compile_fn { echo "Nothing to compile."; }
function test_fn { npm test; }

function solcjs_test
{
    TEST_DIR=$(pwd)
    SOLCJS_INPUT_DIR="$TEST_DIR"/test/externalTests/solc-js

    # set up solc-js on the branch specified
    setup master

    printLog "Updating index.js file..."
    echo "require('./determinism.js');" >> test/index.js

    printLog "Copying determinism.js..."
    cp -f $SOLCJS_INPUT_DIR/determinism.js test/

    printLog "Copying contracts..."
    cp -Rf $SOLCJS_INPUT_DIR/DAO test/

    printLog "Copying SMTChecker tests..."
    cp -Rf "$TEST_DIR"/test/libsolidity/smtCheckerTests test/

    # Update version (needed for some tests)
    echo "Updating package.json to version $VERSION"
    npm version --allow-same-version --no-git-tag-version $VERSION

    run_test compile_fn test_fn
}

external_test solc-js solcjs_test
