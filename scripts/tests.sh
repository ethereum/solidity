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

# There is an implicit assumption here that we HAVE to run from root directory.
REPO_ROOT=$(pwd)

# This conditional is only needed because we don't have a working Homebrew
# install for `eth` at the time of writing, so we unzip the ZIP file locally
# instead.  This will go away soon.
if [[ "$OSTYPE" == "darwin"* ]]; then
    ETH_PATH="$REPO_ROOT/eth"
else
    ETH_PATH="eth"
fi

# This trailing ampersand directs the shell to run the command in the background,
# that is, it is forked and run in a separate sub-shell, as a job,
# asynchronously. The shell will immediately return the return status of 0 for
# true and continue as normal, either processing further commands in a script
# or returning the cursor focus back to the user in a Linux terminal.
$ETH_PATH --test -d /tmp/test &

# Wait until the IPC endpoint is available.  That won't be available instantly.
# The node needs to get a little way into its startup sequence before the IPC
# is available and is ready for the unit-tests to start talking to it.
while [ ! -S /tmp/test/geth.ipc ]; do sleep 2; done

# TODO - It should be possible to set the IPC path with explicit parameters:
# 
#     ./test/soltest --ipc /tmp/test/geth.ipc
#
# But that doesn't work on macOS, so we're just using the cruder approach of
# using an environment variable.  That works on Linux and macOS.   We will
# need to check if this command-line support works for Windows too, when we
# have implemented IPC Sockets support at all for Windows.
export ETH_TEST_IPC=/tmp/test/geth.ipc
$REPO_ROOT/build/test/soltest
ERROR_CODE=$?
pkill eth
exit $ERROR_CODE
