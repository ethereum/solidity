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

FULLARGS="--optimize --ignore-missing --combined-json abi,asm,ast,bin,bin-runtime,compact-format,devdoc,hashes,interface,metadata,opcodes,srcmap,srcmap-runtime,userdoc"
OLDARGS="--optimize --combined-json abi,asm,ast,bin,bin-runtime,devdoc,interface,metadata,opcodes,srcmap,srcmap-runtime,userdoc"
function compileFull()
{
    local expected_exit_code=0
    local expect_output=0
    if [[ $1 = '-e' ]]; then
        expected_exit_code=1
        expect_output=1
        shift;
    fi
    if [[ $1 = '-w' ]]; then
        expect_output=1
        shift;
    fi
    if [[ $1 = '-o' ]]; then
        expect_output=2
        shift;
    fi
    local args=$FULLARGS
    if [[ $1 = '-v' ]]; then
        if (echo $2 | grep -Po '(?<=0.4.)\d+' >/dev/null); then
            patch=$(echo $2 | grep -Po '(?<=0.4.)\d+')
            if (( patch < 22 )); then
                args=$OLDARGS
            fi
        fi
        shift 2
    fi

    local files="$*"
    local output

    local stderr_path=$(mktemp)

    set +e
    "$SOLC" ${args} ${files} >/dev/null 2>"$stderr_path"
    local exit_code=$?
    local errors=$(grep -v -E 'Warning: This is a pre-release compiler version|Warning: Experimental features are turned on|pragma experimental ABIEncoderV2|^ +--> |^ +\||^[0-9]+ +\|' < "$stderr_path")
    set -e
    rm "$stderr_path"

    if [[ \
        ("$exit_code" -ne "$expected_exit_code" || \
            ( $expect_output -eq 0 && -n "$errors" ) || \
            ( $expect_output -ne 0 && $expected_exit_code -eq 0 && $expect_output -ne 2 && -z "$errors" ))
    ]]
    then
        printError "Unexpected compilation result:"
        printError "Expected failure: $expected_exit_code - Expected warning / error output: $expect_output"
        printError "Was failure: $exit_code"
        echo "$errors"
        printError "While calling:"
        echo "\"$SOLC\" $ARGS $files"
        printError "Inside directory:"
        pwd
        false
    fi
}
