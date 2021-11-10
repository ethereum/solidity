#!/usr/bin/env bash

set -eo pipefail

function init_import_tests()
{
  export SOLIDITY_BUILD_DIR=${SOLIDITY_BUILD_DIR:-${REPO_ROOT}/build}
  export SOLC=${SOLIDITY_BUILD_DIR}/solc/solc
  export SPLITSOURCES=${REPO_ROOT}/scripts/splitSources.py
  export FAILED=0
  export UNCOMPILABLE=0
  export TESTED=0

  if [[ "$(find . -maxdepth 0 -type d -empty)" == "" ]]; then
      echo "Test directory not empty. Skipping!"
      exit 1
  fi
}

function run_import_tests()
{
    local TEST_FILES=$1
    local SPLITSOURCES=$2
    local NSOURCES=$3
    local WORKINGDIR=$4

    for solfile in $TEST_FILES; do
        echo -n "."
        # create a temporary sub-directory
        local FILETMP
        FILETMP=$(mktemp -d)
        cd "$FILETMP"

        set +e
        local OUTPUT
        OUTPUT=$("$SPLITSOURCES" "$solfile")
        local SPLITSOURCES_RC=$?
        set -e
        if [ ${SPLITSOURCES_RC} == 0 ]; then
            # echo $OUTPUT
            NSOURCES=$((NSOURCES - 1))
            for i in $OUTPUT; do
                testImportExportEquivalence "$i" "$OUTPUT"
                NSOURCES=$((NSOURCES + 1))
            done
        elif [ ${SPLITSOURCES_RC} == 1 ]; then
            testImportExportEquivalence "$solfile"
        elif [ ${SPLITSOURCES_RC} == 2 ]; then
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

    if [ "$FAILED" = 0 ]; then
        echo "SUCCESS: $TESTED tests passed, $FAILED failed, $UNCOMPILABLE could not be compiled ($NSOURCES sources total)."
    else
        echo "FAILURE: Out of $NSOURCES sources, $FAILED failed, ($UNCOMPILABLE could not be compiled)."
        exit 1
    fi
}
