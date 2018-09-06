#!/usr/bin/env bash

#------------------------------------------------------------------------------
# Bash script to run commandline Solidity tests.
#
# The documentation for solidity is hosted at:
#
#     https://solidity.readthedocs.org
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
# (c) 2016 solidity contributors.
#------------------------------------------------------------------------------

set -e

REPO_ROOT=$(cd $(dirname "$0")/.. && pwd)
echo $REPO_ROOT
SOLC="$REPO_ROOT/build/solc/solc"

FULLARGS="--optimize --ignore-missing --combined-json abi,asm,ast,bin,bin-runtime,compact-format,devdoc,hashes,interface,metadata,opcodes,srcmap,srcmap-runtime,userdoc"

echo "Checking that the bug list is up to date..."
"$REPO_ROOT"/scripts/update_bugs_by_version.py

function printTask() { echo "$(tput bold)$(tput setaf 2)$1$(tput sgr0)"; }

function printError() { echo "$(tput setaf 1)$1$(tput sgr0)"; }

function compileFull()
{
    local expected_exit_code=0
    local expect_output=0
    if [[ $1 = '-e' ]]
    then
        expected_exit_code=1
        expect_output=1
        shift;
    fi
    if [[ $1 = '-w' ]]
    then
        expect_output=1
        shift;
    fi

    local files="$*"
    local output

    local stderr_path=$(mktemp)

    set +e
    "$SOLC" $FULLARGS $files >/dev/null 2>"$stderr_path"
    local exit_code=$?
    local errors=$(grep -v -E 'Warning: This is a pre-release compiler version|Warning: Experimental features are turned on|pragma experimental ABIEncoderV2|\^-------------------------------\^' < "$stderr_path")
    set -e
    rm "$stderr_path"

    if [[ \
        "$exit_code" -ne "$expected_exit_code" || \
            ( $expect_output -eq 0 && -n "$errors" ) || \
            ( $expect_output -ne 0 && -z "$errors" ) \
    ]]
    then
        printError "Unexpected compilation result:"
        printError "Expected failure: $expected_exit_code - Expected warning / error output: $expect_output"
        printError "Was failure: $exit_code"
        echo "$errors"
        printError "While calling:"
        echo "\"$SOLC\" $FULLARGS $files"
        printError "Inside directory:"
        pwd
        false
    fi
}

printTask "Testing unknown options..."
(
    set +e
    output=$("$SOLC" --allow=test 2>&1)
    failed=$?
    set -e

    if [ "$output" == "unrecognised option '--allow=test'" ] && [ $failed -ne 0 ] ; then
	echo "Passed"
    else
	printError "Incorrect response to unknown options: $STDERR"
	exit 1
    fi
)

# General helper function for testing SOLC behaviour, based on file name, compile opts, exit code, stdout and stderr.
# An failure is expected.
test_solc_file_input_failures() {
    local filename="${1}"
    local solc_args="${2}"
    local stdout_expected="${3}"
    local stderr_expected="${4}"
    local stdout_path=`mktemp`
    local stderr_path=`mktemp`

    set +e
    "$SOLC" "${filename}" ${solc_args} 1>$stdout_path 2>$stderr_path
    exitCode=$?
    set -e

    if [[ $exitCode -eq 0 ]]; then
        printError "Incorrect exit code. Expected failure (non-zero) but got success (0)."
        rm -f $stdout_path $stderr_path
        exit 1
    fi

    if [[ "$(cat $stdout_path)" != "${stdout_expected}" ]]; then
        printError "Incorrect output on stderr received. Expected:"
        echo -e "${stdout_expected}"

        printError "But got:"
        cat $stdout_path
        rm -f $stdout_path $stderr_path
        exit 1
    fi

    if [[ "$(cat $stderr_path)" != "${stderr_expected}" ]]; then
        printError "Incorrect output on stderr received. Expected:"
        echo -e "${stderr_expected}"

        printError "But got:"
        cat $stderr_path
        rm -f $stdout_path $stderr_path
        exit 1
    fi

    rm -f $stdout_path $stderr_path
}

printTask "Testing passing files that are not found..."
test_solc_file_input_failures "file_not_found.sol" "" "" "\"file_not_found.sol\" is not found."

printTask "Testing passing files that are not files..."
test_solc_file_input_failures "." "" "" "\".\" is not a valid file."

printTask "Testing passing empty remappings..."
test_solc_file_input_failures "${0}" "=/some/remapping/target" "" "Invalid remapping: \"=/some/remapping/target\"."
test_solc_file_input_failures "${0}" "ctx:=/some/remapping/target" "" "Invalid remapping: \"ctx:=/some/remapping/target\"."

printTask "Compiling various other contracts and libraries..."
(
cd "$REPO_ROOT"/test/compilationTests/
for dir in *
do
    if [ "$dir" != "README.md" ]
    then
        echo " - $dir"
        cd "$dir"
        compileFull -w *.sol */*.sol
        cd ..
    fi
done
)

printTask "Compiling all examples from the documentation..."
SOLTMPDIR=$(mktemp -d)
(
    set -e
    cd "$REPO_ROOT"
    REPO_ROOT=$(pwd) # make it absolute
    cd "$SOLTMPDIR"
    "$REPO_ROOT"/scripts/isolate_tests.py "$REPO_ROOT"/docs/ docs
    for f in *.sol
    do
        # The contributors guide uses syntax tests, but we cannot
        # really handle them here.
        if grep -E 'DeclarationError:|// ----' "$f" >/dev/null
        then
            continue
        fi
        echo "$f"
        opts=''
        # We expect errors if explicitly stated, or if imports
        # are used (in the style guide)
        if grep -E "This will not compile|import \"" "$f" >/dev/null
        then
            opts="-e"
        fi
        if grep "This will report a warning" "$f" >/dev/null
        then
            opts="$opts -w"
        fi
        compileFull $opts "$SOLTMPDIR/$f"
    done
)
rm -rf "$SOLTMPDIR"
echo "Done."

printTask "Testing library checksum..."
echo '' | "$SOLC" - --link --libraries a:0x90f20564390eAe531E810af625A22f51385Cd222 >/dev/null
! echo '' | "$SOLC" - --link --libraries a:0x80f20564390eAe531E810af625A22f51385Cd222 &>/dev/null

printTask "Testing long library names..."
echo '' | "$SOLC" - --link --libraries aveeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeerylonglibraryname:0x90f20564390eAe531E810af625A22f51385Cd222 >/dev/null

printTask "Testing overwriting files..."
SOLTMPDIR=$(mktemp -d)
(
    set -e
    # First time it works
    echo 'contract C {} ' | "$SOLC" - --bin -o "$SOLTMPDIR/non-existing-stuff-to-create" 2>/dev/null
    # Second time it fails
    ! echo 'contract C {} ' | "$SOLC" - --bin -o "$SOLTMPDIR/non-existing-stuff-to-create" 2>/dev/null
    # Unless we force
    echo 'contract C {} ' | "$SOLC" - --overwrite --bin -o "$SOLTMPDIR/non-existing-stuff-to-create" 2>/dev/null
)
rm -rf "$SOLTMPDIR"

printTask "Testing assemble, yul, strict-assembly..."
echo '{}' | "$SOLC" - --assemble &>/dev/null
echo '{}' | "$SOLC" - --yul &>/dev/null
echo '{}' | "$SOLC" - --strict-assembly &>/dev/null

printTask "Testing standard input..."
SOLTMPDIR=$(mktemp -d)
(
    set +e
    output=$("$SOLC" --bin  2>&1)
    result=$?
    set -e

    # This should fail
    if [[ !("$output" =~ "No input files given") || ($result == 0) ]] ; then
        printError "Incorrect response to empty input arg list: $STDERR"
        exit 1
    fi

    set +e
    output=$(echo 'contract C {} ' | "$SOLC" - --bin 2>/dev/null | grep -q "<stdin>:C")
    result=$?
    set -e

    # The contract should be compiled
    if [[ "$result" != 0 ]] ; then
        exit 1
    fi
)

printTask "Testing soljson via the fuzzer..."
SOLTMPDIR=$(mktemp -d)
(
    set -e
    cd "$REPO_ROOT"
    REPO_ROOT=$(pwd) # make it absolute
    cd "$SOLTMPDIR"
    "$REPO_ROOT"/scripts/isolate_tests.py "$REPO_ROOT"/test/
    "$REPO_ROOT"/scripts/isolate_tests.py "$REPO_ROOT"/docs/ docs
    for f in *.sol
    do
        set +e
        "$REPO_ROOT"/build/test/tools/solfuzzer --quiet < "$f"
        if [ $? -ne 0 ]; then
            printError "Fuzzer failed on:"
            cat "$f"
            exit 1
        fi

        "$REPO_ROOT"/build/test/tools/solfuzzer --without-optimizer --quiet < "$f"
        if [ $? -ne 0 ]; then
            printError "Fuzzer (without optimizer) failed on:"
            cat "$f"
            exit 1
        fi
        set -e
    done
)
rm -rf "$SOLTMPDIR"
echo "Commandline tests successful."
