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

if [ "$1" = --junit_report ]
then
    if [ -z "$2" ]
    then
        echo "Usage: $0 [--junit_report <report_directory>]"
        exit 1
    fi
    testargs_no_opt="--logger=JUNIT,test_suite,$2/no_opt.xml"
    testargs_opt="--logger=JUNIT,test_suite,$2/opt.xml"
else
    testargs_no_opt=''
    testargs_opt=''
fi

echo "Running commandline tests..."
"$REPO_ROOT/test/cmdlineTests.sh" &
CMDLINE_PID=$!
# Only run in parallel if this is run on CI infrastructure
if [ -z "$CI" ]
then
    wait $CMDLINE_PID
fi

function download_eth()
{
    if [[ "$OSTYPE" == "darwin"* ]]; then
        ETH_PATH="$REPO_ROOT/eth"
    elif [ -z $CI ]; then
        ETH_PATH="eth"
    else
        mkdir -p /tmp/test
        ETH_BINARY=eth_byzantium_artful
        ETH_HASH="e527dd3e3dc17b983529dd7dcfb74a0d3a5aed4e"
        if grep -i trusty /etc/lsb-release >/dev/null 2>&1
        then
            ETH_BINARY=eth_byzantium2
            ETH_HASH="4dc3f208475f622be7c8e53bee720e14cd254c6f"
        fi
        wget -q -O /tmp/test/eth https://github.com/ethereum/cpp-ethereum/releases/download/solidityTester/$ETH_BINARY
        test "$(shasum /tmp/test/eth)" = "$ETH_HASH  /tmp/test/eth"
        sync
        chmod +x /tmp/test/eth
        sync # Otherwise we might get a "text file busy" error
        ETH_PATH="/tmp/test/eth"
    fi

}

# $1: data directory
# echos the PID
function run_eth()
{
    $ETH_PATH --test -d "$1" >/dev/null 2>&1 &
    echo $!
    # Wait until the IPC endpoint is available.
    while [ ! -S "$1"/geth.ipc ] ; do sleep 1; done
    sleep 2
}

download_eth
ETH_PID=$(run_eth /tmp/test)

progress="--show-progress"
if [ "$CI" ]
then
    progress=""
fi

echo "--> Running tests without optimizer..."
"$REPO_ROOT"/build/test/soltest $testargs_no_opt $progress -- --ipcpath /tmp/test/geth.ipc
echo "--> Running tests WITH optimizer..."
"$REPO_ROOT"/build/test/soltest $testargs_opt $progress -- --optimize --ipcpath /tmp/test/geth.ipc

wait $CMDLINE_PID

pkill "$ETH_PID" || true
sleep 4
pgrep "$ETH_PID" && pkill -9 "$ETH_PID" || true