#!/usr/bin/env bash

# ------------------------------------------------------------------------------
# SPDX-License-Identifier: GPL-3.0
#------------------------------------------------------------------------------
source scripts/common.sh
source test/externalTests/common.sh

verify_input "$1"
SOLJSON="$1"

function install_fn { npm install; }
function compile_fn { npx truffle compile; }
function test_fn { npm test; }

function gnosis_safe_test
{
    OPTIMIZER_LEVEL=1
    CONFIG="truffle.js"

    truffle_setup https://github.com/solidity-external-tests/safe-contracts.git development_060

    force_truffle_version
    sed -i 's|github:gnosis/mock-contract#sol_0_5_0|github:solidity-external-tests/mock-contract#master_060|g' package.json

    run_install install_fn
    replace_libsolc_call

    truffle_run_test compile_fn test_fn
}

external_test Gnosis-Safe gnosis_safe_test
