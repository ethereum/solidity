#!/usr/bin/env bash

set -e

# Bash script to test the ast-import option of the compiler by
# first exporting a .sol file to JSON, then loading it into the compiler
# and exporting it again. The second JSON should be identical to the first
READLINK=readlink
if [[ "$OSTYPE" == "darwin"* ]]; then
    READLINK=greadlink
fi
IMPORT_TEST_TYPE=${1}
REPO_ROOT=$(${READLINK} -f "$(dirname "$0")"/..)
SOLIDITY_BUILD_DIR=${SOLIDITY_BUILD_DIR:-${REPO_ROOT}/build}
SOLC=${SOLIDITY_BUILD_DIR}/solc/solc
SPLITSOURCES=${REPO_ROOT}/scripts/splitSources.py

SYNTAXTESTS_DIR="${REPO_ROOT}/test/libsolidity/syntaxTests"
SEMANTICTESTS_DIR="${REPO_ROOT}/test/libsolidity/semanticTests"
ASTJSONTESTS_DIR="${REPO_ROOT}/test/libsolidity/ASTJSON"

# DEV_DIR="${REPO_ROOT}/../tmp/contracts/"
# NSOURCES="$(find $DEV_DIR -type f | wc -l)" #TODO use find command

FAILED=0
UNCOMPILABLE=0
TESTED=0

if [[ "$(find . -maxdepth 0 -type d -empty)" == "" ]]; then
    echo "Test directory not empty. Skipping!"
    exit 1
fi

# function tests whether exporting and importing again leaves the JSON ast unchanged
# Results are recorded by adding to FAILED or UNCOMPILABLE.
# Also, in case of a mismatch a diff and the respective ASTs are printed
# Expected parameters:
# $1 name of the file to be exported and imported
# $2 any files needed to do so that might be in parent directories
function testImportExportEquivalence {
    local nth_input_file="$1"
    IFS=" " read -r -a all_input_files <<< "$2"

    if $SOLC --bin "$nth_input_file" "${all_input_files[@]}" > /dev/null 2>&1
    then
        ! [[ -e stderr.txt ]] || { echo "stderr.txt already exists. Refusing to overwrite."; exit 1; }

        if [ "${IMPORT_TEST_TYPE}" == "ast" ]
        then
            # save exported json as expected result (silently)
            $SOLC --combined-json ast --pretty-json "$nth_input_file" "${all_input_files[@]}" > expected.json 2> /dev/null
            # import it, and export it again as obtained result (silently)
            if ! $SOLC --import-ast --combined-json ast --pretty-json expected.json > obtained.json 2> stderr.txt
            then
                # For investigating, use exit 1 here so the script stops at the
                # first failing test
                # exit 1
                FAILED=$((FAILED + 1))
                echo -e "ERROR: AST reimport failed for input file $nth_input_file"
                echo
                echo "Compiler stderr:"
                cat ./stderr.txt
                echo
                echo "Compiler stdout:"
                cat ./obtained.json
                return 1
            fi
            set +e
            DIFF="$(diff expected.json obtained.json)"
            set +e
            if [ "$DIFF" != "" ]
            then
                if [ "$DIFFVIEW" == "" ]
                then
                    echo -e "ERROR: JSONS differ for $1: \n $DIFF \n"
                    echo "Expected:"
                    cat ./expected.json
                    echo "Obtained:"
                    cat ./obtained.json
                else
                    # Use user supplied diff view binary
                    $DIFFVIEW expected.json obtained.json
                fi
                FAILED=$((FAILED + 1))
                return 2
            fi
            TESTED=$((TESTED + 1))
            rm expected.json obtained.json
            rm -f stderr.txt
        elif [ "${IMPORT_TEST_TYPE}" == "evm-assembly" ]
        then
            local types=( "asm" "bin" "bin-runtime" "opcodes" "srcmap" "srcmap-runtime" )
            local _TESTED=1
            if ! $SOLC --combined-json bin,bin-runtime,opcodes,asm,srcmap,srcmap-runtime --pretty-json "$nth_input_file" "${all_input_files[@]}" > expected.json 2> expected.error
            then
                printf "\n"
                echo "$nth_input_file"
                cat expected.error
                UNCOMPILABLE=$((UNCOMPILABLE + 1))
                return 0
            else
                for contract in $(jq '.contracts | keys | .[]' expected.json 2> /dev/null)
                do
                    for type in "${types[@]}"
                    do
                        jq --raw-output ".contracts.${contract}.\"${type}\"" expected.json > "expected.${type}"
                    done

                    assembly=$(cat expected.asm)
                    if [ "$assembly" != "" ] && [ "$assembly" != "null" ]
                    then
                        if ! $SOLC --combined-json bin,bin-runtime,opcodes,asm,srcmap,srcmap-runtime --pretty-json --import-asm-json expected.asm > obtained.json 2> obtained.error
                        then
                            printf "\n"
                            echo "$nth_input_file"
                            cat obtained.error
                            FAILED=$((FAILED + 1))
                            return 0
                        else
                            for type in "${types[@]}"
                            do
                                for obtained_contract in $(jq '.contracts | keys | .[]' obtained.json  2> /dev/null)
                                do
                                    jq --raw-output ".contracts.${obtained_contract}.\"${type}\"" obtained.json > "obtained.${type}"
                                    set +e
                                    DIFF="$(diff "expected.${type}" "obtained.${type}")"
                                    set -e
                                    if [ "$DIFF" != "" ]
                                    then
                                        if [ "$DIFFVIEW" == "" ]
                                        then
                                            echo -e "ERROR: JSONS differ for $1: \n $DIFF \n"
                                            echo "Expected:"
                                            cat  "expected.${type}"
                                            echo "Obtained:"
                                            cat "obtained.${type}"
                                        else
                                            # Use user supplied diff view binary
                                            $DIFFVIEW expected.json obtained.json
                                        fi
                                        _TESTED=
                                        FAILED=$((FAILED + 1))
                                        return 0
                                    fi
                                done
                            done

                            # direct export via --asm-json, if imported with --import-asm-json.
                            if ! $SOLC --asm-json --import-asm-json expected.asm | tail -n+4 > obtained_direct_import_export.json 2> obtained_direct_import_export.error
                            then
                                printf "\n"
                                echo "$nth_input_file"
                                cat obtained_direct_import_export.error
                                FAILED=$((FAILED + 1))
                                return 0
                            else
                                for obtained_contract in $(jq '.contracts | keys | .[]' obtained_direct_import_export.json 2> /dev/null)
                                do
                                    jq --raw-output ".contracts.${obtained_contract}.\"asm\"" obtained_direct_import_export.json > obtained_direct_import_export.asm
                                    set +e
                                    DIFF="$(diff expected.asm obtained_direct_import_export.asm)"
                                    set -e
                                    if [ "$DIFF" != "" ]
                                    then
                                        if [ "$DIFFVIEW" == "" ]
                                        then
                                            echo -e "ERROR: JSONS differ for $1: \n $DIFF \n"
                                            echo "Expected:"
                                            cat  expected.asm
                                            echo "Obtained:"
                                            cat obtained_direct_import_export.asm
                                        else
                                            # Use user supplied diff view binary
                                            $DIFFVIEW expected.asm obtained_direct_import_export.asm
                                        fi
                                        _TESTED=
                                        FAILED=$((FAILED + 1))
                                        return 0
                                    fi
                                done
                            fi

                            rm obtained.json
                            rm -f obtained.error
                            for type in "${types[@]}"
                            do
                                rm "obtained.${type}"
                            done
                        fi

                        for type in "${types[@]}"
                        do
                            rm "expected.${type}"
                        done
                    fi
                done
                rm expected.json
            fi
            if [ -n "${_TESTED}" ]
            then
                TESTED=$((TESTED + 1))
            fi
        else
            echo "unknown import test type. aborting."
            exit 1
        fi
    else
        UNCOMPILABLE=$((UNCOMPILABLE + 1))
    fi
}

WORKINGDIR=$PWD
NSOURCES=0

# check whether SOLC works.
if ! $SOLC --version > /dev/null 2>&1
then
    echo "$SOLC not found. aborting."
    exit 1
fi

# check whether jq can be found.
if ! jq --version > /dev/null 2>&1
then
    echo "jq needed. please install. aborting."
    exit 1
fi

# for solfile in $(find $DEV_DIR -name *.sol)
# boost_filesystem_bug specifically tests a local fix for a boost::filesystem
# bug. Since the test involves a malformed path, there is no point in running
# AST tests on it. See https://github.com/boostorg/filesystem/issues/176
if [ "${IMPORT_TEST_TYPE}" == "ast" ]
then
    IMPORT_TEST_FILES=$(find "${SYNTAXTESTS_DIR}" "${ASTJSONTESTS_DIR}" -name "*.sol" -and -not -name "boost_filesystem_bug.sol")
elif [ "${IMPORT_TEST_TYPE}" == "evm-assembly" ]
then
    IMPORT_TEST_FILES=$(find "${SYNTAXTESTS_DIR}" "${SEMANTICTESTS_DIR}" -name "*.sol" -and -not -name "boost_filesystem_bug.sol")
else
    echo "unknown import test type. aborting. please specify $0 [ast|evm-assembly]."
    exit 1
fi

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
        # echo $OUTPUT
        NSOURCES=$((NSOURCES - 1))
        for i in $OUTPUT;
        do
            testImportExportEquivalence "$i" "$OUTPUT"
            NSOURCES=$((NSOURCES + 1))
        done
    elif [ ${SPLITSOURCES_RC} == 1 ]
    then
        testImportExportEquivalence "$solfile"
    elif [ ${SPLITSOURCES_RC} == 2 ]
    then
        # The script will exit with return code 2, if an UnicodeDecodeError occurred.
        # This is the case if e.g. some tests are using invalid utf-8 sequences. We will ignore
        # these errors, but print the actual output of the script.
        echo -e "\n${OUTPUT}\n"
        testImportExportEquivalence "$solfile"
    else
        # All other return codes will be treated as critical errors. The script will exit.
        echo -e "\nGot unexpected return code ${SPLITSOURCES_RC} from ${SPLITSOURCES}. Aborting."
        echo -e "\n${OUTPUT}\n"

        cd "$WORKINGDIR"
        # Delete temporary files
        rm -rf "$FILETMP"

        exit 1
    fi

    cd "$WORKINGDIR"
    # Delete temporary files
    rm -rf "$FILETMP"
done

echo ""

if [ "$FAILED" = 0 ]
then
    echo "SUCCESS: $TESTED tests passed, $FAILED failed, $UNCOMPILABLE could not be compiled ($NSOURCES sources total)."
else
    echo "FAILURE: Out of $NSOURCES sources, $FAILED failed, ($UNCOMPILABLE could not be compiled)."
    exit 1
fi
