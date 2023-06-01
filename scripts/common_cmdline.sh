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

YULARGS=(--strict-assembly)
FULLARGS=(--optimize --combined-json "abi,asm,ast,bin,bin-runtime,devdoc,hashes,metadata,opcodes,srcmap,srcmap-runtime,userdoc")
OLDARGS=(--optimize --combined-json "abi,asm,ast,bin,bin-runtime,devdoc,interface,metadata,opcodes,srcmap,srcmap-runtime,userdoc")

function compileFull
{
    local expected_exit_code=0
    local expect_output='none'

    case "$1" in
        '--expect-errors')
            expected_exit_code=1
            expect_output='warnings-or-errors'
            shift;
            ;;
        '--expect-warnings')
            expect_output='warnings-or-errors'
            shift;
            ;;
        '--ignore-warnings')
            expect_output='any'
            shift;
            ;;
    esac

    local args=("${FULLARGS[@]}")
    if [[ $1 = '-v' ]]; then
        if (echo "$2" | grep -Po '(?<=0.4.)\d+' >/dev/null); then
            patch=$(echo "$2" | grep -Po '(?<=0.4.)\d+')
            if (( patch < 22 )); then
                args=("${OLDARGS[@]}")
            fi
        fi
        shift 2
    fi

    local files=("$@")

    local stderr_path; stderr_path=$(mktemp)

    if [ "${files: -4}" == ".yul" ]
    then
        args=("${YULARGS[@]}")
    fi

    set +e
    "$SOLC" "${args[@]}" "${files[@]}" >/dev/null 2>"$stderr_path"
    local exit_code=$?
    local errors; errors=$(grep -v -E \
        -e 'Warning: This is a pre-release compiler version|Warning: Experimental features are turned on|pragma experimental ABIEncoderV2|^ +--> |^ +\||^[0-9]+ +\| ' \
        -e '^No text representation found.$' < "$stderr_path"
    )

    set -e
    rm "$stderr_path"

    if [[
        $exit_code != "$expected_exit_code" ||
        $errors != "" && $expect_output == 'none' ||
        $errors == "" && $expect_output != 'none' && $expect_output != 'any' && $expected_exit_code == 0
    ]]
    then
        printError "TEST FAILURE"
        printError "Actual exit code:   $exit_code"
        printError "Expected exit code: $expected_exit_code"
        printError "==== Output ===="
        echo "$errors"
        printError "== Output end =="
        printError ""
        case "$expect_output" in
            'none') printError "No output was expected." ;;
            'warnings-or-errors') printError "Expected warnings or errors." ;;
        esac

        printError ""
        printError "While calling:"
        echo      "\"$SOLC\" ${args[*]} ${files[*]}"
        printError "Inside directory:"
        echo "    $(pwd)"
        printError "Input was:"
        echo "${files[@]}"
        false
    fi
}

function singleContractOutputViaStandardJSON
{
    (( $# == 4 )) || assertFail
    local language="$1"
    local selected_output="$2"
    local extra_settings="$3"
    local input_file="$4"
    [[ $selected_output != "*" ]] || assertFail

    json_output=$(
        "$SOLC" --standard-json --allow-paths "$(basename "$input_file")" - <<EOF
        {
            "language": "${language}",
            "sources": {"${input_file}": {"urls": ["${input_file}"]}},
            "settings": {
                "outputSelection": {"*": { "*": ["${selected_output}"]}},
                ${extra_settings}
            }
        }
EOF
    )

    local error_list output has_contract_level_outputs

    error_list=$(echo "$json_output" | jq '.errors[] | select(.severity=="error")')
    [[ $error_list == '' ]] || \
        fail "Failed to compile ${input_file} via Standard JSON. Errors: ${error_list}"

    has_contract_level_outputs=$(echo "$json_output" | jq 'has("contracts")')
    [[ $has_contract_level_outputs == true ]] || \
        fail "Standard JSON output ${selected_output} was ignored by the compiler."

    output=$(echo "$json_output" | jq --raw-output ".contracts[\"${input_file}\"][].${selected_output}")
    [[ $output != null ]] || \
        fail "Compiler failed to produce the ${selected_output} output when compiling ${input_file} via Standard JSON."

    echo "$output"
}

function stripCLIDecorations
{
    sed -e '/^=======.*=======$/d' \
        -e '/^Binary:$/d' \
        -e '/^Binary of the runtime part:$/d' \
        -e '/^Opcodes:$/d' \
        -e '/^IR:$/d' \
        -e '/^Optimized IR:$/d' \
        -e '/^EVM assembly:$/d' \
        -e '/^JSON AST (compact format):$/d' \
        -e '/^Function signatures:$/d' \
        -e '/^Contract Storage Layout:$/d' \
        -e '/^Developer Documentation$/d' \
        -e '/^User Documentation$/d' \
        -e '/^Contract JSON ABI$/d' \
        -e '/^Metadata:$/d' \
        -e '/^EVM$/d' \
        -e '/^Pretty printed source:$/d' \
        -e '/^Text representation:$/d' \
        -e '/^Binary representation:$/d'
}

function stripEmptyLines
{
    sed -e '/^\s*$/d'
}
