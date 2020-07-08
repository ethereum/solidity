#!/usr/bin/env bash

# ------------------------------------------------------------------------------
# SPDX-License-Identifier: GPL-3.0
#------------------------------------------------------------------------------
source scripts/common.sh
source test/externalTests/common.sh

verify_input "$1"
SOLJSON="$1"

function install_fn { yarn; git submodule update --init; }
function compile_fn { yarn run provision:token:contracts; }
function test_fn { yarn run test:contracts; }

function colony_test
{
    OPTIMIZER_LEVEL=3
    FORCE_ABIv2=false
    CONFIG="truffle.js"

    truffle_setup https://github.com/solidity-external-tests/colonyNetwork.git develop_060
    run_install install_fn

    cd lib
    rm -Rf dappsys
    git clone https://github.com/solidity-external-tests/dappsys-monolithic.git -b master_060 dappsys
    cd ..

    truffle_run_test compile_fn test_fn
}

external_test ColonyNetworks colony_test
