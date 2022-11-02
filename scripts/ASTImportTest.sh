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
#   - first a .sol file will be exported to a combined json file, containing outputs
#     for "asm" "bin" "bin-runtime" "opcodes" "srcmap" and "srcmap-runtime" (expected output).
#     The "asm" output will then be used as import, where its output "bin" "bin-runtime"
#     "opcodes" "srcmap" "srcmap-runtime" (obtained output) will be compared with the expected output.
#     The expected output needs to be identical with the obtained output.
#
#     Additionally to this, the direct import/export is tested by importing an
#     evm-assembly json with --import-asm-json and directly exporting it again with
#     --asm-json using the same solc invocation. The original asm json file used for the
#     import and the resulting exported asm json file need to be identical.

set -euo pipefail

READLINK="readlink"
if [[ "${OSTYPE}" == "darwin"* ]]; then
    READLINK="greadlink"
fi
EXPR="expr"
if [[ "${OSTYPE}" == "darwin"* ]]; then
    EXPR="gexpr"
fi

REPO_ROOT=$(${READLINK} -f "$(dirname "$0")"/..)
SOLIDITY_BUILD_DIR=${SOLIDITY_BUILD_DIR:-${REPO_ROOT}/build}
SOLC="${SOLIDITY_BUILD_DIR}/solc/solc"
SPLITSOURCES="${REPO_ROOT}/scripts/splitSources.py"

# shellcheck source=scripts/common.sh
source "${REPO_ROOT}/scripts/common.sh"

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
    printError "You can find the files used for this test here: ${test_directory}"
    printError "Used commands for test:"
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
    printError "${error_message}"
    printError ""
    printError "stderr:"
    cat "${stderr_file}" >&2
    printError ""
    printError "stdout:"
    cat "${stdout_file}" >&2
}

function check_import_test_type_unset
{
    [[ -z "${IMPORT_TEST_TYPE}" ]] || fail "ERROR: Import test type can only be set once. Aborting."
}

IMPORT_TEST_TYPE=
EXIT_ON_ERROR=0
for PARAM in "$@"
do
    case "${PARAM}" in
        ast) check_import_test_type_unset ; IMPORT_TEST_TYPE="ast" ;;
        evm-assembly) check_import_test_type_unset ; IMPORT_TEST_TYPE="evm-assembly" ;;
        --help) print_usage ; exit 0 ;;
        --exit-on-error) EXIT_ON_ERROR=1 ;;
        *) fail "Unknown option '${PARAM}'. Aborting. $(print_usage || true)" ;;
    esac
done

SYNTAXTESTS_DIR="${REPO_ROOT}/test/libsolidity/syntaxTests"
ASTJSONTESTS_DIR="${REPO_ROOT}/test/libsolidity/ASTJSON"
SEMANTICTESTS_DIR="${REPO_ROOT}/test/libsolidity/semanticTests"
YULTESTS_DIR="${REPO_ROOT}/test/libyul"


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

function split_combined_json
{
    local json_file="$1"
    local output_path="$2"
    local prefix="${3:-}"
    for path_with_contract in $(jq '.contracts | keys | .[]' "${json_file}" 2> /dev/null)
    do
        local path=${path_with_contract}
        local contract=""
        local delimiter
        delimiter=$("${EXPR}" index "${path}" ":") || true
        if [[ -z "${prefix}" ]]
        then
            path=${path_with_contract:0:((${delimiter} - 1))}
            contract=${path_with_contract:((${delimiter})):((${#path_with_contract} - ${delimiter} - 1))}
        else
            path=${path_with_contract}
            contract=""
        fi
        for type in $(jq --raw-output ".contracts.${path_with_contract} | keys | .[]" "${json_file}" 2> /dev/null)
        do
            local output
            output=$(jq --raw-output ".contracts.${path_with_contract}.\"${type}\"" "${json_file}")
            if [[ -n "${output}" ]]
            then
              echo "${output}" > "${output_path}/${prefix}${contract}.${type}"
            fi
        done
    done
    rm "${json_file}"
}

function run_solc
{
    local parameters=( "${@}" )

    if ! "${SOLC}" "${parameters[@]}" > /dev/null 2> solc_stderr
    then
        printError "ERROR: ${parameters[*]}"
        printError "${PWD}"
        local SOLC_STDERR
        SOLC_STDERR=$(cat "solc_stderr")
        printError "${SOLC_STDERR}"
        exit 1
    fi
    rm -f solc_stderr

    return 0
}

function run_solc_store_stdout
{
    local output_file=$1
    local parameters=( "${@:2}" )

    if ! "${SOLC}" "${parameters[@]}" > "${output_file}" 2> "${output_file}.error"
    then
        printError "ERROR: ${parameters[*]}"
        printError "${PWD}"
        local SOLC_STDERR
        SOLC_STDERR=$(cat "${output_file}.error")
        printError "${SOLC_STDERR}"
        exit 1
    fi
    rm -f "${output_file}.error"

    return 0
}

function test_evmjson_via_ir_and_yul_import_export
{
    local sol_file="$1"
    local input_files=( "${@:2}" )

    mkdir yul
    # export found solidity contracts to yul.
    run_solc --optimize --via-ir --ir-optimized "${input_files[@]}" --no-optimize-yul  -o yul/
    for filename in yul/*
    do
        if [[ -s "${filename}" ]]
        then
          # remove '_opt' part of '<contract-name>_opt.yul'
          mv "${filename}" "${filename/_opt/}"
        else
          # if file was empty, delete it.
          rm -f "${filename}"
        fi
    done

    local export_command
    local import_command

    mkdir sol
    # create from a combined json from the supplied solidity contracts.
    export_command=("${SOLC}" --combined-json "bin,bin-runtime,opcodes,asm,srcmap,srcmap-runtime" --optimize --via-ir --pretty-json --json-indent 4 --no-optimize-yul "${input_files[@]}" -o sol/)
    run_solc --combined-json "bin,bin-runtime,opcodes,asm,srcmap,srcmap-runtime" --optimize --via-ir --pretty-json --json-indent 4 "${input_files[@]}" --no-optimize-yul  -o sol/
    mkdir input

    # save the original supplied solidity contract sources for potential debugging purposes.
    for file in "${input_files[@]}"
    do
        cat "${file}" >> "input/$(basename "${file}")"
    done
    # split the combined json into different files.
    split_combined_json sol/combined.json sol/

    # iterate through all yul files.
    for yulfile in yul/*
    do
        # take the yul file and export it as evm assembly json. save the result in "$yulfile.asm.json".
        run_solc_store_stdout "${yulfile}.asm.json" --strict-assembly "${yulfile}" --optimize --asm-json --pretty-json --json-indent 4 --no-optimize-yul
        # remove the lines containing '=======', so that we just have a nice json file.
        grep -v '^=======' "${yulfile}.asm.json" > tmpfile && mv tmpfile "${yulfile}.asm.json"
        # import_command will just contain the last file in yul/*.asm - but it's still a good starting point ;)
        import_command=("${SOLC}" --combined-json "bin,bin-runtime,opcodes,asm,srcmap,srcmap-runtime" --pretty-json --json-indent 4 --import-asm-json "${yulfile}.asm.json")
        # import the created evm assembly json file and create a combined json out of it.
        run_solc_store_stdout "${yulfile}.combined.json" --combined-json "bin,bin-runtime,opcodes,asm,srcmap,srcmap-runtime" --pretty-json --json-indent 4 --import-asm-json "${yulfile}.asm.json"
        # split the combined json into different files.
        split_combined_json "${yulfile}.combined.json" . "${yulfile}"
    done

    # now iterate over all files in the sol/ output folder.
    # the files stored here will normally look like e.g. "sol/C.asm", "sol/C.bin"
    # they were generated by the split_combined_json call above and contain the contract
    # name and the type (e.g. bin,bin-runtime,opcodes,asm,srcmap,srcmap-runtime) in the
    # file-name.
    for file in sol/*
    do
        local type
        local delimiter
        local yul

        # first just remove all path parts from the file and put it in the type.
        # - "sol/C.asm" -> "C.asm"
        type="$(basename "${file}")"

        # delimiter is just the position of that dot that is delimiting the contract
        # name from it's type.
        delimiter=$("${EXPR}" index "${type}" ".")

        # extract the type: for e.g. "C.asm" -> type will be "asm".
        type=${type:((${delimiter})):((${#type} - ${delimiter}))}

        # now we want to know which is the corresponding yul file, that should have exactly
        # the same content. e.g. file="sol/C.srcmap-runtime" -> yul="yul/C.yul.srcmap-runtime"
        yul=${file/sol/yul}
        yul=${yul/${type}/yul.${type}}

        # remember that we first exported the yul file from solidity contract files.
        # then we generated and split the corresponding combined-json of that exported yul file.
        # we also generated and split another combined-json file from the original
        # solidity contract files. if the yul export to asm evm json and it's re-import
        # will lead to the same content of the combined-json of the original solidity file,
        # the yul export to asm evm json and it's reimport seem to work correctly.

        # we can ignore "asm" and "json" files here. "asm" is the exported evm asm json.
        # between the yul and the sol assembly jsons we may have some subtile differences,
        # e.g. in the source-lists.
        # however, if the yul/sol outputs of e.g. bin,bin-runtime,opcodes,srcmap,srcmap-runtime
        # is matching, we consider that the reimport was done correctly.
        if [[ "${type}" == "asm" ]] || [[ "${type}" == "json" ]]
        then
            continue
        fi

        # compare the files. e.g. "sol/C.srcmap-runtime" with "yul/C.yul.srcmap-runtime"
        if ! diff_files "${file}" "${yul}" > diff_error
        then
            local diff_error
            diff_error=$(cat diff_error)
            printError "ERROR: diff failed ${file} ${yul}:\n   ${diff_error}"
            if (( EXIT_ON_ERROR == 1 ))
            then
                print_used_commands "${PWD}" "${export_command[*]}" "${import_command[*]}"
                exit 1
            fi
            return 1
        fi
    done

    rm -rf sol
    rm -rf yul
    rm -rf input

    return 0
}

function test_evmjson_sol_import_export
{
    local sol_file="$1"
    local input_files=( "${@:2}" )

    mkdir -p sol
    # create from a combined json from the supplied solidity contracts.
    local export_command
    local import_command
    export_command=("${SOLC}" --combined-json "bin,bin-runtime,opcodes,asm,srcmap,srcmap-runtime" --pretty-json --json-indent 4 "${input_files[@]}" -o sol/)

    run_solc --combined-json "bin,bin-runtime,opcodes,asm,srcmap,srcmap-runtime" --pretty-json --json-indent 4 "${input_files[@]}" -o sol/
    mkdir input
    # save the original supplied solidity contract sources for potential debugging purposes.
    for file in "${input_files[@]}"
    do
        cat "${file}" >> "input/$(basename "${file}")"
    done
    # split the combined json into different files.
    split_combined_json sol/combined.json sol/

    mkdir -p imported-from-sol
    for file in sol/*.asm
    do
        name=$(basename "${file}" .asm)
        # import_command will just contain the last file in sol/*.asm - but it's still a good starting point ;)
        import_command=("${SOLC}" --import-ast --combined-json "bin,bin-runtime,opcodes,asm,srcmap,srcmap-runtime" --pretty-json --json-indent 4 --import-asm-json "${file}")
        run_solc_store_stdout "imported-from-sol/combined.json" --combined-json "bin,bin-runtime,opcodes,asm,srcmap,srcmap-runtime" --pretty-json --json-indent 4 --import-asm-json "${file}"
        split_combined_json imported-from-sol/combined.json imported-from-sol/ "${name}"
    done

    for file in sol/*
    do
        local imported
        imported=${file/sol/imported-from-sol}
        if ! diff_files "${file}" "${imported}" > diff_error
        then
            local diff_error
            diff_error=$(cat diff_error)
            printError "ERROR: diff failed ${file} ${imported}:\n   ${diff_error}"
            if (( EXIT_ON_ERROR == 1 ))
            then
                print_used_commands "${PWD}" "${export_command[*]}" "${import_command[*]}"
                exit 1
            fi
            return 1
        fi
    done

    rm -rf sol
    rm -rf imported-from-sol
    rm -rf input

    return 0
}

function test_evmjson_import_export_equivalence
{
    local sol_file="$1"
    local input_files=( "${@:2}" )
    local success=1

    # export sol to yul. generate artefacts from sol and convert yul to asm json.
    # import the yul asm json and check whether the sol artefacts are the same as for yul.
    if ! test_evmjson_via_ir_and_yul_import_export "${sol_file}" "${input_files[@]}"
    then
        success=0
    fi

    # only run the next test-step, if the previous test was run correctly.
    if (( success == 1 ))
    then
        # generate artefacts from sol. export sol to asm json. import that asm json and
        # check whether the sol artefacts are the same as created by the asm json import.
        if ! test_evmjson_sol_import_export "${sol_file}" "${input_files[@]}"
        then
            success=0
        fi
    fi

    if (( success == 1 ))
    then
        TESTED=$((TESTED + 1))
    else
        FAILED=$((FAILED + 1))
    fi
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

    case "${IMPORT_TEST_TYPE}" in
        ast) compile_test="--ast-compact-json" ;;
        evm-assembly) compile_test="--bin" ;;
        *) assertFail "Unknown import test type '${IMPORT_TEST_TYPE}'. Aborting." ;;
    esac

    set +e
    output=$("${SOLC}" "${compile_test}" "${input_files[@]}" 2>&1)
    solc_return_code=$?
    set -e

    # if input files where compilable with success
    if (( solc_return_code == 0 ))
    then
        case "${IMPORT_TEST_TYPE}" in
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
            # For the evm-assembly import/export tests, this script uses only the
            # old code generator. Some semantic test can only be compiled with
            # --via-ir (some need to be additionally compiled with --optimize).
            # The tests that are meant to be compiled with --via-ir are throwing
            # an UnimplementedFeatureError exception (e.g. Copying of type struct C.S
            # memory[] memory to storage not yet supported, Copying nested calldata
            # dynamic arrays to storage is not implemented in the old code generator.)
            # We will just ignore these kind of exceptions for now.
            # However, any other exception will be treated as a fatal error and the
            # script execution will be terminated with an error.
            if [[ "${output}" != *"UnimplementedFeatureError"* ]]
            then
                fail "\n\nERROR: Uncaught Exception while executing '${SOLC} ${compile_test} ${input_files[*]}':\n${output}\n"
            fi
        fi
    fi
}

function test_evmjson_import_from_yul_export
{
    local files="${*}"
    for yulfile in $files
    do
        local export_command
        local import_command
        echo -n "·"

        # create a temporary sub-directory
        FILETMP=$(mktemp -d)
        cd "${FILETMP}"

        set +e
        output=$("${SOLC}" "--strict-assembly" "${yulfile}" "--bin" "--optimize" 2>/dev/null)
        solc_return_code=$?
        set -e

        # if yul file got compiled with success.
        if (( solc_return_code == 0 ))
        then
            yul_bin="$(basename "$yulfile").bin"
            yul_json="$(basename "$yulfile").asm.json"
            echo "$output" > "${yul_bin}"
            # remove all lines starting with '=======' and 'Binary representation:'.
            grep -v -e '^=======' -e '^Binary representation:' "${yul_bin}" > tmpfile && mv tmpfile "${yul_bin}"
            # remove all white-spaces. we only want the binary.
            tr -d '[:space:]' < "${yul_bin}" > tmpfile && mv tmpfile "${yul_bin}"
            # only compare bytecode, if bytecode got generated.
            if [[ -s "${yul_bin}" ]]
            then
                # take the yul file and export it as evm assembly json. save the result in "$yul_json".
                export_command=("${SOLC}" --strict-assembly "${yulfile}" --optimize --asm-json --pretty-json --json-indent 4)
                run_solc_store_stdout "${yul_json}" --strict-assembly "${yulfile}" --optimize --asm-json --pretty-json --json-indent 4
                # remove the lines containing '=======', so that we just have a nice json file.
                grep -v '^=======' "${yul_json}" > tmpfile && mv tmpfile "${yul_json}"

                # import the created evm assembly json file and create a combined json out of it.
                import_command=("${SOLC}" --combined-json "bin" --pretty-json --json-indent 4 --import-asm-json "${yul_json}")
                run_solc_store_stdout "${yul_json}.combined.json" --combined-json "bin" --pretty-json --json-indent 4 --import-asm-json "${yul_json}"
                # split the combined json into different files.
                split_combined_json "${yul_json}.combined.json" . "compiled"
                tr -d '[:space:]' < "compiled.bin" > tmpfile && mv tmpfile "compiled.bin"

                if ! diff_files compiled.bin "${yul_bin}" > diff_error
                then
                    diff_error=$(cat diff_error)
                    printError "ERROR: diff failed ${yulfile}:\n   ${diff_error}"
                    if (( EXIT_ON_ERROR == 1 ))
                    then
                        print_used_commands "${PWD}" "${export_command[*]}" "${import_command[*]}"
                        exit 1
                    fi
                    FAILED=$((FAILED + 1))
                else
                    TESTED=$((TESTED + 1))
                fi
            fi
        fi
        cd "${WORKINGDIR}"
        # Delete temporary files
        rm -rf "${FILETMP}"
    done
    echo
}

WORKINGDIR=${PWD}

command_available "${SOLC}" --version
command_available jq --version
command_available "${EXPR}" --version
command_available "${READLINK}" --version

case "${IMPORT_TEST_TYPE}" in
    ast) TEST_DIRS=("${SYNTAXTESTS_DIR}" "${ASTJSONTESTS_DIR}") ;;
    evm-assembly) TEST_DIRS=("${SEMANTICTESTS_DIR}") ;;
    *) assertFail "Import test type not defined. $(print_usage || true)}" ;;
esac

YUL_NSOURCES=0
if [[ "${IMPORT_TEST_TYPE}" == "evm-assembly" ]]
then
    IMPORT_TEST_FILES=$(find "${YULTESTS_DIR}" -name "*.yul")
    YUL_NSOURCES="$(echo "${IMPORT_TEST_FILES}" | wc -l)"
    echo "Looking at ${YUL_NSOURCES} .yul files..."
    test_evmjson_import_from_yul_export "${IMPORT_TEST_FILES[@]}"
fi

# boost_filesystem_bug specifically tests a local fix for a boost::filesystem
# bug. Since the test involves a malformed path, there is no point in running
# tests on it. See https://github.com/boostorg/filesystem/issues/176
IMPORT_TEST_FILES=$(find "${TEST_DIRS[@]}" -name "*.sol" -and -not -name "boost_filesystem_bug.sol" -not -path "*/experimental/*")

SOL_NSOURCES="$(echo "${IMPORT_TEST_FILES}" | wc -l)"
echo "Looking at ${SOL_NSOURCES} .sol files..."

NSOURCES=$(( YUL_NSOURCES + SOL_NSOURCES ))

for solfile in ${IMPORT_TEST_FILES}
do
    echo -n "·"
    # create a temporary sub-directory
    FILETMP=$(mktemp -d)
    cd "${FILETMP}"

    set +e
    OUTPUT=$("${SPLITSOURCES}" "${solfile}")
    SPLITSOURCES_RC=$?
    set -e

    if (( SPLITSOURCES_RC == 0 ))
    then
        IFS=' ' read -ra OUTPUT_ARRAY <<< "${OUTPUT}"
        test_import_export_equivalence "${solfile}" "${OUTPUT_ARRAY[@]}"
    elif (( SPLITSOURCES_RC == 1 ))
    then
        test_import_export_equivalence "${solfile}" "${solfile}"
    elif (( SPLITSOURCES_RC == 2 ))
    then
        # The script will exit with return code 2, if an UnicodeDecodeError occurred.
        # This is the case if e.g. some tests are using invalid utf-8 sequences. We will ignore
        # these errors, but print the actual output of the script.
        printError "\n\n${OUTPUT}\n\n"
        test_import_export_equivalence "${solfile}" "${solfile}"
    else
        # All other return codes will be treated as critical errors. The script will exit.
        printError "\n\nGot unexpected return code ${SPLITSOURCES_RC} from '${SPLITSOURCES} ${solfile}'. Aborting."
        printError "\n\n${OUTPUT}\n\n"

        cd "${WORKINGDIR}"
        # Delete temporary files
        rm -rf "${FILETMP}"

        exit 1
    fi

    cd "${WORKINGDIR}"
    # Delete temporary files
    rm -rf "${FILETMP}"
done

echo

if (( FAILED == 0 ))
then
    echo "SUCCESS: ${TESTED} tests passed, ${FAILED} failed, ${UNCOMPILABLE} could not be compiled (${NSOURCES} sources total)."
else
    fail "FAILURE: Out of ${NSOURCES} sources, ${FAILED} failed, (${UNCOMPILABLE} could not be compiled)."
fi
