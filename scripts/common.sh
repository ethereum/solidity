#!/usr/bin/env bash
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

# The fail() function defined below requires set -e to be enabled.
set -e

# Save the initial working directory so that printStackTrace() can access it even if the sourcing
# changes directory. The paths returned by `caller` are relative to it.
_initial_work_dir=$(pwd)

if [[ -n ${CIRCLECI:-} ]]
then
    export TERM="${TERM:-xterm}"
    function printTask { echo -e "$(tput bold)$(tput setaf 2)$1$(tput setaf 7)"; }
    function printError { >&2 echo -e "$(tput setaf 1)$1$(tput setaf 7)"; }
    function printWarning { >&2 echo -e "$(tput setaf 11)$1$(tput setaf 7)"; }
    function printLog { echo -e "$(tput setaf 3)$1$(tput setaf 7)"; }
else
    function printTask { echo -e "$(tput bold)$(tput setaf 2)$1$(tput sgr0)"; }
    function printError { >&2 echo -e "$(tput setaf 1)$1$(tput sgr0)"; }
    function printWarning { >&2 echo -e "$(tput setaf 11)$1$(tput sgr0)"; }
    function printLog { echo -e "$(tput setaf 3)$1$(tput sgr0)"; }
fi

function checkDputEntries
{
    local pattern="$1"
    grep "${pattern}" /etc/dput.cf --quiet || \
        fail "Error: Missing ${pattern//\\/} section in /etc/dput.cf (check top comment in release_ppa.sh for more information)."
}

function sourcePPAConfig
{
    [[ "$LAUNCHPAD_KEYID" == "" && "$LAUNCHPAD_EMAIL" == "" ]] || fail

    # source keyid and email from .release_ppa_auth
    if [[ -e .release_ppa_auth ]]
    then
        # shellcheck source=/dev/null
        source "${REPO_ROOT}/.release_ppa_auth"
    fi

    [[ "$LAUNCHPAD_KEYID" != "" && "$LAUNCHPAD_EMAIL" != "" ]] || \
        fail "Error: Couldn't find variables \$LAUNCHPAD_KEYID or \$LAUNCHPAD_EMAIL in sourced file .release_ppa_auth (check top comment in $0 for more information)."
}

function printStackTrace
{
    printWarning ""
    printWarning "Stack trace:"

    local frame=1
    while caller "$frame" > /dev/null
    do
        local lineNumber line file function

        # `caller` returns something that could already be printed as a stacktrace but we can make
        # it more readable by rearranging the components.
        # NOTE: This assumes that paths do not contain spaces.
        lineNumber=$(caller "$frame" | cut -d " " -f 1)
        function=$(caller "$frame" | cut -d " " -f 2)
        file=$(caller "$frame" | cut -d " " -f 3)

        # Paths in the output from `caller` can be relative or absolute (depends on how the path
        # with which the script was invoked) and if they're relative, they're not necessarily
        # relative to the current working dir. This is a heuristic that will work if they're absolute,
        # relative to current dir, or relative to the dir that was current when the script started.
        # If neither works, it gives up.
        line=$(
            {
                tail "--lines=+${lineNumber}" "$file" ||
                tail "--lines=+${lineNumber}" "${_initial_work_dir}/${file}"
            } 2> /dev/null |
            head --lines=1 |
            sed -e 's/^[[:space:]]*//'
        ) || line="<failed to find source line>"

        >&2 printf "    %s:%d in function %s()\n" "$file" "$lineNumber" "$function"
        >&2 printf "        %s\n" "$line"

        ((frame++))
    done
}

function fail
{
    printError "$@"

    # Using return rather than exit lets the invoking code handle the failure by suppressing the exit code.
    return 1
}

function assertFail
{
    printError ""
    (( $# == 0 )) && printError "Assertion failed."
    (( $# == 1 )) && printError "Assertion failed: $1"
    printStackTrace

    # Intentionally using exit here because assertion failures are not supposed to be handled.
    exit 2
}

function msg_on_error
{
    local error_message
    local no_stdout=false
    local no_stderr=false

    while [[ $1 =~ ^-- ]]
    do
        case "$1" in
            --msg)
                error_message="$2"
                shift
                shift
                ;;
            --no-stdout)
                no_stdout=true
                shift
                ;;
            --no-stderr)
                no_stderr=true
                shift
                ;;
            --silent)
                no_stdout=true
                no_stderr=true
                shift
                ;;
            *)
                assertFail "Invalid option for msg_on_error: $1"
                ;;
        esac
    done

    local command=("$@")

    local stdout_file stderr_file
    stdout_file="$(mktemp -t cmdline_test_command_stdout_XXXXXX.txt)"
    stderr_file="$(mktemp -t cmdline_test_command_stderr_XXXXXX.txt)"

    if "${command[@]}" > "$stdout_file" 2> "$stderr_file"
    then
        [[ $no_stdout == "true" ]] || cat "$stdout_file"
        [[ $no_stderr == "true" ]] || >&2 cat "$stderr_file"
        rm "$stdout_file" "$stderr_file"
        return 0
    else
        printError ""
        printError "Command failed: ${error_message}"
        printError "    command: $SOLC ${command[*]}"
        if [[ -s "$stdout_file" ]]
        then
            printError "--- stdout ---"
            printError "-----------"
            >&2 cat "$stdout_file"
            printError "--------------"
        else
            printError "    stdout: <EMPTY>"
        fi
        if [[ -s "$stderr_file" ]]
        then
            printError "--- stderr ---"
            >&2 cat "$stderr_file"
            printError "--------------"
        else
            printError "    stderr: <EMPTY>"
        fi

        rm "$stdout_file" "$stderr_file"

        printStackTrace
        return 1
    fi
}


function diff_values
{
    (( $# >= 2 )) || fail "diff_values requires at least 2 arguments."

    local value1="$1"
    local value2="$2"
    shift 2

    if ! diff --unified=0 <(echo "$value1") <(echo "$value2") "$@"
    then
        if [[ -n ${DIFFVIEW:-} ]]
        then
            # Use user supplied diff view binary
            "$DIFFVIEW" <(echo "$value1") <(echo "$value2")
        fi
        return 1
    fi
}

function diff_files
{
    (( $# >= 2 )) || fail "diff_files requires at least 2 arguments."

    local file1="$1"
    local file2="$2"

    if ! diff "${file1}" "${file2}"
    then
        if [[ -n ${DIFFVIEW:-} ]]
        then
            # Use user supplied diff view binary
            "$DIFFVIEW" "${file1}" "${file2}"
        fi
        return 1
    fi
}

function safe_kill
{
    local PID=${1}
    local NAME=${2:-${1}}
    local n=1

    # only proceed if $PID does exist
    kill -0 "$PID" 2>/dev/null || return

    echo "Sending SIGTERM to ${NAME} (${PID}) ..."
    kill "$PID"

    # wait until process terminated gracefully
    while kill -0 "$PID" 2>/dev/null && [[ $n -le 4 ]]; do
        echo "Waiting ($n) ..."
        sleep 1
        n=$((n + 1))
    done

    # process still alive? then hard-kill
    if kill -0 "$PID" 2>/dev/null; then
        echo "Sending SIGKILL to ${NAME} (${PID}) ..."
        kill -9 "$PID"
    fi
}

function circleci_select_steps
{
    # We expect multiple lines in $all_steps, one step per line
    local all_steps="$1"
    (( $# == 1 )) || assertFail

    if (( CIRCLE_NODE_TOTAL )) && (( CIRCLE_NODE_TOTAL > 1 ))
    then
        echo "$all_steps" | circleci tests split | xargs
    else
        echo "$all_steps" | xargs
    fi
}

function circleci_select_steps_multiarg
{
    # We expect multiple arguments, one step per argument.
    circleci_select_steps "$(printf '%s\n' "$@")"
}

function circleci_step_selected
{
    local selected_steps="$1"
    local step="$2"
    (( $# == 2 )) || assertFail
    [[ $step != *" "* ]] || assertFail "Step names must not contain spaces."

    [[ " $selected_steps " == *" $step "* ]] || return 1
}

function first_word
{
    local words="$1"
    (( $# == 1 )) || assertFail

    echo "$words" | cut -d " " -f 1
}

# Function reads from stdin. Therefore it has to be used with pipes.
function split_on_empty_lines_into_numbered_files
{
    path_prefix="${1}"
    path_suffix="${2}"

    awk -v RS= "{print > (\"${path_prefix}_\"NR \"${path_suffix}\")}"
}

function command_available
{
    local program="$1"
    local parameters=${*:2}
    if ! "${program}" "${parameters}" > /dev/null 2>&1
    then
        fail "'${program}' not found or not executed successfully with parameter(s) '${parameters}'. aborting."
    fi
}
