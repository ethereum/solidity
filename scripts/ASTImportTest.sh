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
# Bash script to test the import/exports.
#
# ast import/export tests:
#   - first exporting a .sol file to JSON, then loading it into the compiler
#     and exporting it again. The second JSON should be identical to the first.
#
# evm-assembly import/export tests:
#   - first a .sol file will be compiled and the EVM Assembly will be exported
#     to JSON format using --asm-json command-line option.
#     The EVM Assembly JSON output is then imported with --import-asm-json
#     and compiled again. The binary generated initially and after the import
#     should be identical.

set -euo pipefail

READLINK=readlink
if [[ "$OSTYPE" == "darwin"* ]]; then
    READLINK=greadlink
fi
EXPR="expr"
if [[ "$OSTYPE" == "darwin"* ]]; then
    EXPR="gexpr"
fi

REPO_ROOT=$(${READLINK} -f "$(dirname "$0")"/..)
SOLIDITY_BUILD_DIR=${SOLIDITY_BUILD_DIR:-${REPO_ROOT}/build}
SOLC="${SOLIDITY_BUILD_DIR}/solc/solc"
SPLITSOURCES="${REPO_ROOT}/scripts/splitSources.py"

# shellcheck source=scripts/common.sh
source "${REPO_ROOT}/scripts/common.sh"
# shellcheck source=scripts/common_cmdline.sh
source "${REPO_ROOT}/scripts/common_cmdline.sh"

function print_usage
{
    echo "Usage: ${0} ast|evm-assembly [--exit-on-error|--help]."
}

function print_used_commands
{
    local test_directory="$1"
    local export_command="$2"
    local import_command="$3"
    echo
    printError "You can find the files used for this test in ${test_directory}"
    printError "Commands used in the test:"
    printError "# export"
    echo "$ ${export_command}" >&2
    printError "# import"
    echo "$ ${import_command}" >&2
}

function print_stderr_stdout
{
    local error_message="$1"
    local stderr_file="$2"
    local stdout_file="$3"
    printError "$error_message"
    printError ""
    printError "stderr:"
    cat "$stderr_file" >&2
    printError ""
    printError "stdout:"
    cat "$stdout_file" >&2
}

function check_import_test_type_unset
{
    [[ -z "$IMPORT_TEST_TYPE" ]] || fail "ERROR: Import test type can only be set once. Aborting."
}

IMPORT_TEST_TYPE=
EXIT_ON_ERROR=0
for PARAM in "$@"
do
    case "$PARAM" in
        ast) check_import_test_type_unset ; IMPORT_TEST_TYPE="ast" ;;
        evm-assembly) check_import_test_type_unset ; IMPORT_TEST_TYPE="evm-assembly" ;;
        --help) print_usage ; exit 0 ;;
        --exit-on-error) EXIT_ON_ERROR=1 ;;
        *) fail "Unknown option '$PARAM'. Aborting. $(print_usage)" ;;
    esac
done

SYNTAXTESTS_DIR="${REPO_ROOT}/test/libsolidity/syntaxTests"
ASTJSONTESTS_DIR="${REPO_ROOT}/test/libsolidity/ASTJSON"
SEMANTICTESTS_DIR="${REPO_ROOT}/test/libsolidity/semanticTests"

FAILED=0
UNCOMPILABLE=0
TESTED=0

function test_ast_import_export_equivalence
{
    local sol_file="$1"
    local input_files=( "${@:2}" )

    local export_command=("$SOLC" --combined-json ast --pretty-json --json-indent 4 "${input_files[@]}")
    local import_command=("$SOLC" --import-ast --combined-json ast --pretty-json --json-indent 4 expected.json)
    local import_via_standard_json_command=("$SOLC" --combined-json ast --pretty-json --json-indent 4 --standard-json standard_json_input.json)

    # export ast - save ast json as expected result (silently)
    if ! "${export_command[@]}" > expected.json 2> stderr_export.txt
    then
        print_stderr_stdout "ERROR: AST reimport failed (export) for input file ${sol_file}." ./stderr_export.txt ./expected.json
        print_used_commands "$(pwd)" "${export_command[*]} > expected.json" "${import_command[*]}"
        return 1
    fi

    # (re)import ast - and export it again as obtained result (silently)
    if ! "${import_command[@]}" > obtained.json 2> stderr_import.txt
    then
        print_stderr_stdout "ERROR: AST reimport failed (import) for input file ${sol_file}." ./stderr_import.txt ./obtained.json
        print_used_commands "$(pwd)" "${export_command[*]} > expected.json" "${import_command[*]}"
        return 1
    fi

    echo ". += {\"sources\":" > _ast_json.json
    jq .sources expected.json >> _ast_json.json
    echo "}" >> _ast_json.json
    echo "{\"language\": \"SolidityAST\", \"settings\": {\"outputSelection\": {\"*\": {\"\": [\"ast\"]}}}}" > standard_json.json
    jq --from-file _ast_json.json standard_json.json > standard_json_input.json

    # (re)import ast via standard json - and export it again as obtained result (silently)
    if ! "${import_via_standard_json_command[@]}" > obtained_standard_json.json 2> stderr_import.txt
    then
        print_stderr_stdout "ERROR: AST reimport failed (import) for input file ${sol_file}." ./stderr_import.txt ./obtained_standard_json.json
        print_used_commands "$(pwd)" "${export_command[*]} > expected.json" "${import_command[*]}"
        return 1
    fi

    jq .sources expected.json > expected_standard_json.json
    jq .sources obtained_standard_json.json >  obtained_standard_json_.json
    jq 'walk(if type == "object" and has("ast") then .AST = .ast | del(.ast) else . end)' < obtained_standard_json_.json > obtained_standard_json.json
    jq --sort-keys . < obtained_standard_json.json > obtained_standard_json_.json
    mv obtained_standard_json_.json obtained_standard_json.json

    # compare expected and obtained ASTs
    if ! diff_files expected.json obtained.json || ! diff_files expected_standard_json.json obtained_standard_json.json
    then
        printError "ERROR: AST reimport failed for ${sol_file}"
        if (( EXIT_ON_ERROR == 1 ))
        then
            print_used_commands "$(pwd)" "${export_command[*]}" "${import_command[*]}"
            return 1
        fi
        FAILED=$((FAILED + 1))
    fi
    TESTED=$((TESTED + 1))
}

function run_solc
{
    local parameters=( "${@}" )

    if ! "${SOLC}" "${parameters[@]}" > /dev/null 2> solc_stderr
    then
        printError "ERROR: ${parameters[*]}"
        printError "${PWD}"
        # FIXME: EXIT_ON_ERROR seems to be ignored here and in some other places.
        # We just exit unconditionally instead.
        fail "$(cat solc_stderr)"
    fi
    rm solc_stderr
}

function run_solc_store_stdout
{
    local output_file=$1
    local parameters=( "${@:2}" )

    if ! "${SOLC}" "${parameters[@]}" > "${output_file}" 2> "${output_file}.error"
    then
        printError "ERROR: ${parameters[*]}"
        printError "${PWD}"
        fail "$(cat "${output_file}.error")"
    fi
    rm "${output_file}.error"
}

function test_evmjson_import_export_equivalence
{
    local sol_file="$1"
    local input_files=( "${@:2}" )

    # Generate bytecode and EVM assembly JSON through normal complication
    mkdir -p export/
    local export_options=(--bin --asm-json "${input_files[@]}" --output-dir export/)
    run_solc "${export_options[@]}"

    # NOTE: If there is no bytecode, the compiler produces a JSON file that contains just 'null'.
    # This is not accepted by assembly import though so we must skip such contracts.
    echo -n null > null
    find export/ -name '*.json' -exec cmp --quiet --bytes=4 {} null \; -delete

    find export/ -name '*.bin' -size 0 -delete

    for asm_json_file in export/*.json
    do
        mv "${asm_json_file}" "${asm_json_file/_evm/}"
    done

    # Import EVM assembly JSON
    mkdir -p import/
    for asm_json_file in export/*.json
    do
        local bin_file_from_asm_import
        bin_file_from_asm_import="import/$(basename "${asm_json_file}" .json).bin"

        local import_options=(--bin --import-asm-json "${asm_json_file}")
        run_solc_store_stdout "${bin_file_from_asm_import}" "${import_options[@]}"

        stripCLIDecorations < "$bin_file_from_asm_import" > tmpfile
        mv tmpfile "$bin_file_from_asm_import"
    done

    # Compare bytecode from compilation with bytecode from import
    for bin_file in export/*.bin
    do
        local bin_file_from_asm_import=${bin_file/export/import}
        if ! diff --strip-trailing-cr --ignore-all-space "${bin_file}" "${bin_file_from_asm_import}" > diff_error
        then
            printError "ERROR: Bytecode from compilation (${bin_file}) differs from bytecode from EVM asm import (${bin_file_from_asm_import}):"
            printError "    $(cat diff_error)"
            if (( EXIT_ON_ERROR == 1 ))
            then
                # NOTE: The import_options we print here refers to the wrong file (the last one
                # processed by the previous loop) - but it's still a good starting point for debugging ;)
                print_used_commands "${PWD}" "${SOLC} ${export_options[*]}" "${SOLC} ${import_options[*]}"
                return 1
            fi
            FAILED=$((FAILED + 1))
        fi
    done
    TESTED=$((TESTED + 1))
}

# function tests whether exporting and importing again is equivalent.
# Results are recorded by incrementing the FAILED or UNCOMPILABLE global variable.
# Also, in case of a mismatch a diff is printed
# Expected parameters:
# $1 name of the file to be exported and imported
# $2 any files needed to do so that might be in parent directories
function test_import_export_equivalence {
    local sol_file="$1"
    local input_files=( "${@:2}" )
    local output
    local solc_return_code
    local compile_test

    case "$IMPORT_TEST_TYPE" in
        ast) compile_test="--ast-compact-json" ;;
        evm-assembly) compile_test="--bin" ;;
        *) assertFail "Unknown import test type '${IMPORT_TEST_TYPE}'. Aborting." ;;
    esac

    set +e
    output=$("$SOLC" "${compile_test}" "${input_files[@]}" 2>&1)
    solc_return_code=$?
    set -e

    # if input files where compilable with success
    if (( solc_return_code == 0 ))
    then
        case "$IMPORT_TEST_TYPE" in
            ast) test_ast_import_export_equivalence "${sol_file}" "${input_files[@]}" ;;
            evm-assembly) test_evmjson_import_export_equivalence "${sol_file}" "${input_files[@]}" ;;
            *) assertFail "Unknown import test type '${IMPORT_TEST_TYPE}'. Aborting." ;;
        esac
    else
        UNCOMPILABLE=$((UNCOMPILABLE + 1))

        # solc will return exit code 2, if it was terminated by an uncaught exception.
        # This should normally not happen, so we terminate the test execution here
        # and print some details about the corresponding solc invocation.
        if (( solc_return_code == 2 ))
        then
            # For the evm-assembly import/export tests, this script uses only the old code generator.
            # Some semantic tests can only be compiled with --via-ir (some need to be additionally
            # compiled with --optimize). The tests that are meant to be compiled with --via-ir are
            # throwing an UnimplementedFeatureError exception, e.g.:
            # "Copying of type struct C.S memory[] memory to storage not yet supported",
            # "Copying nested calldata dynamic arrays to storage is not implemented in the old code generator".
            # We will just ignore these kind of exceptions for now. However, any other exception
            # will be treated as a fatal error and the script execution will be terminated with an error.
            if [[ "${output}" != *"UnimplementedFeatureError"* ]]
            then
                fail "\n\nERROR: Uncaught exception while executing '${SOLC} ${compile_test} ${input_files[*]}':\n${output}\n"
            fi
        fi
    fi
}

command_available "$SOLC" --version
command_available jq --version
command_available "$EXPR" --version
command_available "$READLINK" --version

case "$IMPORT_TEST_TYPE" in
    ast) TEST_DIRS=("${SYNTAXTESTS_DIR}" "${ASTJSONTESTS_DIR}") ;;
    evm-assembly) TEST_DIRS=("${SEMANTICTESTS_DIR}") ;;
    *) assertFail "Import test type not defined. $(print_usage)" ;;
esac

# boost_filesystem_bug specifically tests a local fix for a boost::filesystem
# bug. Since the test involves a malformed path, there is no point in running
# tests on it. See https://github.com/boostorg/filesystem/issues/176
IMPORT_TEST_FILES=$(find "${TEST_DIRS[@]}" -name "*.sol" -and -not -name "boost_filesystem_bug.sol" -not -path "*/experimental/*")

NSOURCES="$(echo "${IMPORT_TEST_FILES}" | wc -l)"
echo "Looking at ${NSOURCES} .sol files..."

COUNTER=0
TEST_DIR=$(mktemp -d -t "import-export-test-XXXXXX")
pushd "$TEST_DIR" > /dev/null

for solfile in $IMPORT_TEST_FILES
do
    echo -n "·"

    TEST_SUBDIR="$(printf "%05d" "$COUNTER")-$(basename "$solfile")"
    mkdir "$TEST_SUBDIR"
    pushd "$TEST_SUBDIR" > /dev/null

    set +e
    OUTPUT=$("$SPLITSOURCES" "$solfile")
    SPLITSOURCES_RC=$?
    set -e

    if (( SPLITSOURCES_RC == 0 ))
    then
        IFS=' ' read -ra OUTPUT_ARRAY <<< "$OUTPUT"
        test_import_export_equivalence "$solfile" "${OUTPUT_ARRAY[@]}"
    elif (( SPLITSOURCES_RC == 1 ))
    then
        test_import_export_equivalence "$solfile" "$solfile"
    elif (( SPLITSOURCES_RC == 2 ))
    then
        # The script will exit with return code 2, if an UnicodeDecodeError occurred.
        # This is the case if e.g. some tests are using invalid utf-8 sequences. We will ignore
        # these errors, but print the actual output of the script.
        printError "\n\n${OUTPUT}\n\n"
        test_import_export_equivalence "$solfile" "$solfile"
    else
        # All other return codes will be treated as critical errors. The script will exit.
        printError "\n\nGot unexpected return code ${SPLITSOURCES_RC} from '${SPLITSOURCES} ${solfile}'. Aborting."
        printError "\n\n${OUTPUT}\n\n"
        exit 1
    fi

    popd > /dev/null
    ((++COUNTER))
done

popd > /dev/null
rm -r "$TEST_DIR"

echo

if (( FAILED == 0 ))
then
    echo "SUCCESS: ${TESTED} tests passed, ${FAILED} failed, ${UNCOMPILABLE} could not be compiled (${NSOURCES} sources total)."
else
    fail "FAILURE: Out of ${NSOURCES} sources, ${FAILED} failed, (${UNCOMPILABLE} could not be compiled)."
fi
