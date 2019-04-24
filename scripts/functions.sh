# ------------------------------------------------------------------------------
# This file is to be included by other scripts that want to include functionality exported below.
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
# (c) 2019 solidity contributors.
#------------------------------------------------------------------------------

# Default to xterm if no terminal was set.
export TERM=${TERM:-"xterm"}

if [[ "$CIRCLECI" ]]; then
    function printTask() { echo "$(tput bold)$(tput setaf 2)$1$(tput setaf 7)"; }
    function printError() { echo "$(tput setaf 1)$1$(tput setaf 7)"; }
else
    function printTask() { echo "$(tput bold)$(tput setaf 2)$1$(tput sgr0)"; }
    function printError() { echo "$(tput setaf 1)$1$(tput sgr0)"; }
fi

function get_valgrind_prefix()
{
    if [[ "${ENABLE_VALGRIND}" != "true" ]]; then
        return
    fi
    if [[ "$CIRCLECI" ]]; then
        apt-get install -qqy valgrind 1>&2
    fi
    local exe=`which valgrind 2>/dev/null`
    if [[ -z "${exe}" ]]; then
        return
    fi
    echo -n "${exe} -v --num-callers=64 --error-exitcode=42"
}
VG=`get_valgrind_prefix`

function ask_expectation_update()
{
    local newExpectation="${1}"
    local expectationFile="${2}"
    while true;
    do
        set +e
        read -t10 -p "(u)pdate expectation/(q)uit? "
        if [ $? -gt 128 ];
        then
            echo -e "\nUser input timed out."
            exit 1
        fi
        set -e
        case $REPLY in
            u* ) echo "$newExpectation" > $expectationFile ; break;;
            q* ) exit 1;;
        esac
    done
}

function safe_kill()
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

