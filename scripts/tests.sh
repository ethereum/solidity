#!/usr/bin/env bash

#------------------------------------------------------------------------------
# Bash script to execute the Solidity tests.
#
# The documentation for solidity is hosted at:
#
#     https://solidity.readthedocs.org
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

REPO_ROOT="$(dirname "$0")"/..

IPC_ENABLED=true
if [[ "$OSTYPE" == "darwin"* ]]
then
    SMT_FLAGS="--no-smt"
    if [ "$CIRCLECI" ]
    then
        IPC_ENABLED=false
        IPC_FLAGS="--no-ipc"
    fi
fi

if [ "$1" = --junit_report ]
then
    if [ -z "$2" ]
    then
        echo "Usage: $0 [--junit_report <report_directory>]"
        exit 1
    fi
    log_directory="$2"
else
    log_directory=""
fi

function printError() { echo "$(tput setaf 1)$1$(tput sgr0)"; }
function printTask() { echo "$(tput bold)$(tput setaf 2)$1$(tput sgr0)"; }

printTask "Running commandline tests..."
"$REPO_ROOT/test/cmdlineTests.sh" &
CMDLINE_PID=$!
# Only run in parallel if this is run on CI infrastructure
if [ -z "$CI" ]
then
    if ! wait $CMDLINE_PID
    then
        printError "Commandline tests FAILED"
        exit 1
    fi
fi

function download_aleth()
{
    if [[ "$OSTYPE" == "darwin"* ]]; then
        ALETH_PATH="$REPO_ROOT/aleth"
    elif [ -z $CI ]; then
        ALETH_PATH="aleth"
    else
        mkdir -p /tmp/test
        if grep -i trusty /etc/lsb-release >/dev/null 2>&1
        then
            # built from d661ac4fec0aeffbedcdc195f67f5ded0c798278 at 2018-06-20
            ALETH_BINARY=aleth_2018-06-20_trusty
            ALETH_HASH="54b8a5455e45b295e3a962f353ff8f1580ed106c"
        else
            # built from d661ac4fec0aeffbedcdc195f67f5ded0c798278 at 2018-06-20
            ALETH_BINARY=aleth_2018-06-20_artful
            ALETH_HASH="02e6c4b3d98299885e73f7db6c9e3fbe3d66d444"
        fi
        ALETH_PATH="/tmp/test/aleth"
        wget -q -O $ALETH_PATH https://github.com/ethereum/cpp-ethereum/releases/download/solidityTester/$ALETH_BINARY
        test "$(shasum $ALETH_PATH)" = "$ALETH_HASH  $ALETH_PATH"
        sync
        chmod +x $ALETH_PATH
        sync # Otherwise we might get a "text file busy" error
    fi

}

# $1: data directory
# echos the PID
function run_aleth()
{
    $ALETH_PATH --test -d "$1" >/dev/null 2>&1 &
    echo $!
    # Wait until the IPC endpoint is available.
    while [ ! -S "$1"/geth.ipc ] ; do sleep 1; done
    sleep 2
}

function check_aleth() {
    printTask "Running IPC tests with $ALETH_PATH..."
    if ! hash $ALETH_PATH 2>/dev/null; then
      printError "$ALETH_PATH not found"
      exit 1
    fi
}

if [ "$IPC_ENABLED" = true ];
then
    download_aleth
    check_aleth
    ALETH_PID=$(run_aleth /tmp/test)
fi

progress="--show-progress"
if [ "$CIRCLECI" ]
then
    progress=""
fi

EVM_VERSIONS="homestead byzantium"

if [ "$CIRCLECI" ] || [ -z "$CI" ]
then
EVM_VERSIONS+=" constantinople"
fi

# And then run the Solidity unit-tests in the matrix combination of optimizer / no optimizer
# and homestead / byzantium VM, # pointing to that IPC endpoint.
for optimize in "" "--optimize"
do
  for vm in $EVM_VERSIONS
  do
    printTask "--> Running tests using "$optimize" --evm-version "$vm"..."
    log=""
    if [ -n "$log_directory" ]
    then
      if [ -n "$optimize" ]
      then
        log=--logger=JUNIT,test_suite,$log_directory/opt_$vm.xml $testargs
      else
        log=--logger=JUNIT,test_suite,$log_directory/noopt_$vm.xml $testargs_no_opt
      fi
    fi
    "$REPO_ROOT"/build/test/soltest $progress $log -- --testpath "$REPO_ROOT"/test "$optimize" --evm-version "$vm" $SMT_FLAGS $IPC_FLAGS  --ipcpath /tmp/test/geth.ipc
  done
done

if ! wait $CMDLINE_PID
then
    printError "Commandline tests FAILED"
    exit 1
fi

if [ "$IPC_ENABLED" = true ]
then
    pkill "$ALETH_PID" || true
    sleep 4
    pgrep "$ALETH_PID" && pkill -9 "$ALETH_PID" || true
fi
