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
function test_fn { npm run test; }

function zeppelin_test
{
    OPTIMIZER_LEVEL=1
    CONFIG="truffle-config.js"

    truffle_setup https://github.com/OpenZeppelin/openzeppelin-contracts.git master
    run_install install_fn

    truffle_run_test compile_fn test_fn
}

external_test Zeppelin zeppelin_test
