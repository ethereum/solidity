#!/usr/bin/env bash

# Bash script to test the ast-import option of the compiler by
# first exporting a .sol file to JSON, then loading it into the compiler
# and exporting it again. The second JSON should be identical to the first

REPO_ROOT=$(realpath "$(dirname "$0")"/..)
SOLIDITY_BUILD_DIR=${SOLIDITY_BUILD_DIR:-build}
SOLC=${REPO_ROOT}/${SOLIDITY_BUILD_DIR}/solc/solc
SPLITSOURCES=${REPO_ROOT}/scripts/splitSources.py

SYNTAXTESTS_DIR="${REPO_ROOT}/test/libsolidity/syntaxTests"
NSOURCES="$(find $SYNTAXTESTS_DIR -type f | wc -l)"

# DEV_DIR="${REPO_ROOT}/../tmp/contracts/"
# NSOURCES="$(find $DEV_DIR -type f | wc -l)" #TODO use find command

FAILED=0
UNCOMPILABLE=0
TESTED=0

if [ $(ls | wc -l) -ne 0 ]; then
	echo "Test directory not empty. Skipping!"
	exit -1
fi

# function tests whether exporting and importing again leaves the JSON ast unchanged
# Results are recorded by adding to FAILED or UNCOMPILABLE.
# Also, in case of a mismatch a diff and the respective ASTs are printed
# Expected parameters:
# $1 name of the file to be exported and imported
# $2 any files needed to do so that might be in parent directories
function testImportExportEquivalence {
    if $SOLC $1 $2 > /dev/null 2>&1
    then
        # save exported json as expected result (silently)
        $SOLC --combined-json ast,compact-format --pretty-json $1 $2> expected.json 2> /dev/null
        # import it, and export it again as obtained result (silently)
        $SOLC --import-ast --combined-json ast,compact-format --pretty-json expected.json > obtained.json 2> /dev/null
        if [ $? -ne 0 ]
        then
               # For investigating, use exit 1 here so the script stops at the
               # first failing test
               # exit 1
               FAILED=$((FAILED + 1))
               return 1
        fi
        DIFF="$(diff expected.json obtained.json)"
        if [ "$DIFF" != "" ]
        then
            if [ "$DIFFVIEW" == "" ]
            then
                echo -e "ERROR: JSONS differ for $1: \n $DIFF \n"
                echo "Expected:"
                echo "$(cat ./expected.json)"
                echo "Obtained:"
                echo "$(cat ./obtained.json)"
            else
                # Use user supplied diff view binary
                $DIFFVIEW expected.json obtained.json
            fi
            FAILED=$((FAILED + 1))
            return 2
        fi
        TESTED=$((TESTED + 1))
        rm expected.json obtained.json
    else
        # echo "contract $solfile could not be compiled "
        UNCOMPILABLE=$((UNCOMPILABLE + 1))
    fi
    # return 0
}
echo "Looking at $NSOURCES .sol files..."

WORKINGDIR=$PWD

# for solfile in $(find $DEV_DIR -name *.sol)
for solfile in $(find $SYNTAXTESTS_DIR -name *.sol)
do
    echo -n "."
    # create a temporary sub-directory
    FILETMP=$(mktemp -d -p $WORKINGDIR)
    cd $FILETMP

    OUTPUT=$($SPLITSOURCES $solfile)
    if [ $? != 1 ]
    then
        # echo $OUTPUT
        NSOURCES=$((NSOURCES - 1))
        for i in $OUTPUT;
        do
            testImportExportEquivalence $i $OUTPUT
            NSOURCES=$((NSOURCES + 1))
        done

    else
        testImportExportEquivalence $solfile
    fi

    cd $WORKINGDIR
    # Delete temporary files
    rm -rf $FILETMP
done

echo ""

if [ "$FAILED" = 0 ]
then
    echo "SUCCESS: $TESTED syntaxTests passed, $FAILED failed, $UNCOMPILABLE could not be compiled ($NSOURCES sources total)."
else
    echo "FAILURE: Out of $NSOURCES sources, $FAILED failed, ($UNCOMPILABLE could not be compiled)."
    exit 1
fi
