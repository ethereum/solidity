# ------------------------------------------------------------------------------
# vim:ts=4:et
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
# (c) 2016-2019 solidity contributors.
# ------------------------------------------------------------------------------

if [ "$CIRCLECI" ]
then
    export TERM="${TERM:-xterm}"
    function printTask() { echo "$(tput bold)$(tput setaf 2)$1$(tput setaf 7)"; }
    function printError() { echo "$(tput setaf 1)$1$(tput setaf 7)"; }
    function printLog() { echo "$(tput setaf 3)$1$(tput setaf 7)"; }
else
    function printTask() { echo "$(tput bold)$(tput setaf 2)$1$(tput sgr0)"; }
    function printError() { echo "$(tput setaf 1)$1$(tput sgr0)"; }
    function printLog() { echo "$(tput setaf 3)$1$(tput sgr0)"; }
fi

safe_kill()
{
    local PID=${1}
    local NAME=${2:-${1}}
    local n=1

    # only proceed if $PID does exist
    kill -0 $PID 2>/dev/null || return

    echo "Sending SIGTERM to ${NAME} (${PID}) ..."
    kill $PID

    # wait until process terminated gracefully
    while kill -0 $PID 2>/dev/null && [[ $n -le 4 ]]; do
        echo "Waiting ($n) ..."
        sleep 1
        n=$[n + 1]
    done

    # process still alive? then hard-kill
    if kill -0 $PID 2>/dev/null; then
        echo "Sending SIGKILL to ${NAME} (${PID}) ..."
        kill -9 $PID
    fi
}

# Ensures aleth executable and exposes the following information:
#
#  - env var ALETH_PATH: to point to the aleth executable.
#  - directory /tmp/test if needed. No cleanup is done on this directory
function download_aleth()
{
    if which aleth &>/dev/null; then
        ALETH_PATH=`which aleth`
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        ALETH_PATH="$(realpath $(dirname "$0")/..)/aleth"
    elif [ "$CIRCLECI" ] || [ -z $CI ]; then
        ALETH_PATH="aleth"
    else
        ALETH_PATH="/tmp/test/bin/aleth"
        mkdir -p /tmp/test
        # Any time the hash is updated here, the "Running the compiler tests" section in contributing.rst should also be updated.
        local ALETH_HASH="7f7004e1563299bc57882e32b32e4a195747dfb6"
        local ALETH_VERSION="1.6.0"
        wget -q -O /tmp/test/aleth.tar.gz https://github.com/ethereum/aleth/releases/download/v${ALETH_VERSION}/aleth-${ALETH_VERSION}-linux-x86_64.tar.gz
        test "$(shasum /tmp/test/aleth.tar.gz)" = "$ALETH_HASH  /tmp/test/aleth.tar.gz"
        tar -xf /tmp/test/aleth.tar.gz -C /tmp/test
        sync
        chmod +x $ALETH_PATH
        sync # Otherwise we might get a "text file busy" error
    fi
}

# Executes aleth in the background and echos its PID.
function run_aleth()
{
    $ALETH_PATH --db memorydb --test -d "${WORKDIR}" &> /dev/null &
    echo $!

    # Wait until the IPC endpoint is available.
    while [ ! -S "${WORKDIR}/geth.ipc" ] ; do sleep 1; done
    sleep 2
}

