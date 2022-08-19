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
# (c) solidity contributors.
# ------------------------------------------------------------------------------

set -euo pipefail
IMPORT_TEST_TYPE="${1}"

# Bash script to test the import/exports.
# ast import/export tests:
#   - first exporting a .sol file to JSON, then loading it into the compiler
#     and exporting it again. The second JSON should be identical to the first.
# evm-assembly import/export tests:
#   - <TODO>

READLINK=readlink
if [[ "$OSTYPE" == "darwin"* ]]; then
    READLINK=greadlink
fi
REPO_ROOT=$(${READLINK} -f "$(dirname "$0")"/..)
SOLIDITY_BUILD_DIR=${SOLIDITY_BUILD_DIR:-${REPO_ROOT}/build}
SOLC=${SOLIDITY_BUILD_DIR}/solc/solc
SPLITSOURCES=${REPO_ROOT}/scripts/splitSources.py

# shellcheck source=scripts/common.sh
source "${REPO_ROOT}/scripts/common.sh"

SYNTAXTESTS_DIR="${REPO_ROOT}/test/libsolidity/syntaxTests"
SEMANTICTESTS_DIR="${REPO_ROOT}/test/libsolidity/semanticTests"
ASTJSONTESTS_DIR="${REPO_ROOT}/test/libsolidity/ASTJSON"

FAILED=0
UNCOMPILABLE=0
TESTED=0

if [[ "$(find . -maxdepth 0 -type d -empty)" == "" ]]; then
    fail "Test directory not empty. Skipping!"
fi

function Ast_ImportExportEquivalence
{
    local sol_file="$1"
    local input_files="$2"
    # save exported json as expected result (silently)
    $SOLC --combined-json ast --pretty-json --json-indent 4 "${input_files}" > expected.json 2> /dev/null
    # import it, and export it again as obtained result (silently)
    if ! $SOLC --import-ast --combined-json ast --pretty-json --json-indent 4 expected.json > obtained.json 2> stderr.txt
    then
        # For investigating, use exit 1 here so the script stops at the
        # first failing test
        # exit 1
        FAILED=$((FAILED + 1))
        printError -e "ERROR: AST reimport failed for input file $sol_file"
        printError
        printError "Compiler stderr:"
        cat ./stderr.txt >&2
        printError
        printError "Compiler stdout:"
        cat ./obtained.json >&2
        return 1
    fi
    if ! diff_files expected.json obtained.json
    then
        FAILED=$((FAILED + 1))
    fi
    TESTED=$((TESTED + 1))
    rm expected.json obtained.json
    rm -f stderr.txt
}

function JsonEvmAsm_ImportExportEquivalence
{
    local sol_file="$1"
    local input_files="$2"
    local outputs=( "asm" "bin" "bin-runtime" "opcodes" "srcmap" "srcmap-runtime" )
    local _TESTED=1
    if ! "${SOLC}" --combined-json "$(IFS=, ; echo "${outputs[*]}")" --pretty-json --json-indent 4 "${input_files}" > expected.json 2> expected.error
    then
        printError
        printError "$sol_file"
        cat expected.error >&2
        UNCOMPILABLE=$((UNCOMPILABLE + 1))
        return 0
    fi

    # Note that we have some test files, that only consists of free functions.
    # Those files doesn't define any contracts, so the resulting json does not define any
    # keys. In this case `jq` returns an error like `jq: error: null (null) has no keys`
    # to not get spammed by these errors, errors are redirected to /dev/null.
    for contract in $(jq '.contracts | keys | .[]' expected.json 2> /dev/null)
    do
        for output in "${outputs[@]}"
        do
            jq --raw-output ".contracts.${contract}.\"${output}\"" expected.json > "expected.${output}.json"
        done

        assembly=$(cat expected.asm.json)
        if [ "$assembly" != "" ] && [ "$assembly" != "null" ]
        then
            if ! "${SOLC}" --combined-json bin,bin-runtime,opcodes,asm,srcmap,srcmap-runtime --pretty-json --json-indent 4 --import-asm-json expected.asm.json > obtained.json 2> obtained.error
            then
                printError
                printError "$sol_file"
                cat obtained.error >&2
                FAILED=$((FAILED + 1))
                return 0
            fi

            for output in "${outputs[@]}"
            do
                for obtained_contract in $(jq '.contracts | keys | .[]' obtained.json  2> /dev/null)
                do
                    jq --raw-output ".contracts.${obtained_contract}.\"${output}\"" obtained.json > "obtained.${output}.json"
                    if ! diff_files "expected.${output}.json" "obtained.${output}.json"
                    then
                        _TESTED=
                        FAILED=$((FAILED + 1))
                        return 0
                    fi
                done
            done

            # direct export via --asm-json, if imported with --import-asm-json.
            if ! "${SOLC}" --asm-json --import-asm-json expected.asm.json --pretty-json --json-indent 4 | tail -n+4 > obtained_direct_import_export.json 2> obtained_direct_import_export.error
            then
                printError
                printError "$sol_file"
                cat obtained_direct_import_export.error >&2
                FAILED=$((FAILED + 1))
                return 0
            fi

            # reformat jsons using jq.
            jq . expected.asm.json > expected.asm.json.pretty
            jq . obtained_direct_import_export.json > obtained_direct_import_export.json.pretty
            if ! diff_files expected.asm.json.pretty obtained_direct_import_export.json.pretty
            then
                _TESTED=
                FAILED=$((FAILED + 1))
                return 0
            fi
        fi
    done

    if [ -n "${_TESTED}" ]
    then
        TESTED=$((TESTED + 1))
    fi
}

# function tests whether exporting and importing again leaves the JSON ast unchanged
# Results are recorded by adding to FAILED or UNCOMPILABLE.
# Also, in case of a mismatch a diff and the respective ASTs are printed
# Expected parameters:
# $1 name of the file to be exported and imported
# $2 any files needed to do so that might be in parent directories
function testImportExportEquivalence {
    local sol_file="$1"
    local input_files="$2"
    if $SOLC --bin "${input_files}" > /dev/null 2>&1
    then
        ! [[ -e stderr.txt ]] || { fail "stderr.txt already exists. Refusing to overwrite."; }

        if [[ $IMPORT_TEST_TYPE == "ast" ]]
        then
            Ast_ImportExportEquivalence "${sol_file}" "${input_files}"
        elif [[ $IMPORT_TEST_TYPE == "evm-assembly" ]]
        then
            JsonEvmAsm_ImportExportEquivalence "${sol_file}" "${input_files}"
        else
            fail "Unknown import test type. Aborting."
        fi
    else
        UNCOMPILABLE=$((UNCOMPILABLE + 1))
    fi
}

WORKINGDIR=$PWD
NSOURCES=0

command_available "${SOLC}" --version
command_available jq --version

# for solfile in $(find $DEV_DIR -name *.sol)
# boost_filesystem_bug specifically tests a local fix for a boost::filesystem
# bug. Since the test involves a malformed path, there is no point in running
# AST tests on it. See https://github.com/boostorg/filesystem/issues/176
if [[ $IMPORT_TEST_TYPE == "ast" ]]
then
    TEST_DIRS=("${SYNTAXTESTS_DIR}" "${ASTJSONTESTS_DIR}")
elif [[ $IMPORT_TEST_TYPE == "evm-assembly" ]]
then
    TEST_DIRS=("${SYNTAXTESTS_DIR}" "${SEMANTICTESTS_DIR}")
else
    fail "Unknown import test type. Aborting. Please specify ${0} [ast|evm-assembly]."
fi

IMPORT_TEST_FILES=$(find "${TEST_DIRS[@]}" -name "*.sol" -and -not -name "boost_filesystem_bug.sol")

NSOURCES="$(echo "$IMPORT_TEST_FILES" | wc -l)"
echo "Looking at $NSOURCES .sol files..."

for solfile in ${IMPORT_TEST_FILES}
do
    echo -n "."
    # create a temporary sub-directory
    FILETMP=$(mktemp -d)
    cd "$FILETMP"

    set +e
    OUTPUT=$("$SPLITSOURCES" "$solfile")
    SPLITSOURCES_RC=$?
    set -e
    if [ ${SPLITSOURCES_RC} == 0 ]
    then
        OIFS=${IFS}
        IFS=' ' read -ra OUTPUT_ARRAY <<< "${OUTPUT}"
        IFS=${OIFS}
        NSOURCES=$((NSOURCES - 1 + ${#OUTPUT_ARRAY[@]}))
        testImportExportEquivalence "$solfile" "${OUTPUT[@]}"
    elif [ ${SPLITSOURCES_RC} == 1 ]
    then
        testImportExportEquivalence "$solfile" "$solfile"
    elif [ ${SPLITSOURCES_RC} == 2 ]
    then
        # The script will exit with return code 2, if an UnicodeDecodeError occurred.
        # This is the case if e.g. some tests are using invalid utf-8 sequences. We will ignore
        # these errors, but print the actual output of the script.
        echo >&2
        printError "\n${OUTPUT[*]}\n"
        echo >&2
        testImportExportEquivalence "$solfile" "$solfile"
    else
        # All other return codes will be treated as critical errors. The script will exit.
        echo >&2
        printError "\nGot unexpected return code ${SPLITSOURCES_RC} from ${SPLITSOURCES}. Aborting."
        echo >&2
        printError "\n${OUTPUT[*]}\n"
        echo >&2

        cd "$WORKINGDIR"
        # Delete temporary files
        rm -rf "$FILETMP"

        exit 1
    fi

    cd "$WORKINGDIR"
    # Delete temporary files
    rm -rf "$FILETMP"
done

echo

if (( FAILED == 0 ))
then
    echo "SUCCESS: $TESTED tests passed, $FAILED failed, $UNCOMPILABLE could not be compiled ($NSOURCES sources total)."
else
    fail "FAILURE: Out of $NSOURCES sources, $FAILED failed, ($UNCOMPILABLE could not be compiled)."
fi
