#!/usr/bin/env bash

#------------------------------------------------------------------------------
# Bash script to run commandline Solidity tests.
#
# The documentation for solidity is hosted at:
#
#     https://docs.soliditylang.org
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

## GLOBAL VARIABLES

REPO_ROOT=$(cd "$(dirname "$0")/.." && pwd)
SOLIDITY_BUILD_DIR=${SOLIDITY_BUILD_DIR:-${REPO_ROOT}/build}
# shellcheck source=scripts/common.sh
source "${REPO_ROOT}/scripts/common.sh"
# shellcheck source=scripts/common_cmdline.sh
source "${REPO_ROOT}/scripts/common_cmdline.sh"

(( $# <= 1 )) || { printError "Too many arguments"; exit 1; }
(( $# == 0 )) || [[ $1 == '--update' ]] || { printError "Invalid argument: '$1'"; exit 1; }

AUTOUPDATE=false
[[ $1 == --update ]] && AUTOUPDATE=true

case "$OSTYPE" in
    msys)
        SOLC="${SOLIDITY_BUILD_DIR}/solc/Release/solc.exe"

        # prevents msys2 path translation for a remapping test
        export MSYS2_ARG_CONV_EXCL="="
        ;;
    *)
        SOLC="${SOLIDITY_BUILD_DIR}/solc/solc"
        ;;
esac
echo "${SOLC}"

INTERACTIVE=true
if ! tty -s || [ "$CI" ]
then
    INTERACTIVE=""
fi

# extend stack size in case we run via ASAN
if [[ -n "${CIRCLECI}" ]] || [[ -n "$CI" ]]; then
    ulimit -s 16384
    ulimit -a
fi

## FUNCTIONS

function update_expectation {
    local newExpectation="${1}"
    local expectationFile="${2}"

    echo "$newExpectation" > "$expectationFile"
    printLog "File $expectationFile updated to match the expectation."
}

function ask_expectation_update
{
    if [[ $INTERACTIVE != "" ]]
    then
        local newExpectation="${1}"
        local expectationFile="${2}"

        if [[ $AUTOUPDATE == true ]]
        then
            update_expectation "$newExpectation" "$expectationFile"
        else
            local editor="${FCEDIT:-${VISUAL:-${EDITOR:-vi}}}"

            while true
            do
                read -r -n 1 -p "(e)dit/(u)pdate expectations/(s)kip/(q)uit? "
                echo
                case $REPLY in
                    e*) "$editor" "$expectationFile"; break;;
                    u*) update_expectation "$newExpectation" "$expectationFile"; break;;
                    s*) return;;
                    q*) exit 1;;
                esac
            done
        fi
    else
        exit 1
    fi
}

# General helper function for testing SOLC behaviour, based on file name, compile opts, exit code, stdout and stderr.
# An failure is expected.
function test_solc_behaviour()
{
    local filename="${1}"
    local solc_args
    IFS=" " read -r -a solc_args <<< "${2}"
    local solc_stdin="${3}"
    [ -z "$solc_stdin"  ] && solc_stdin="/dev/stdin"
    local stdout_expected="${4}"
    local exit_code_expected="${5}"
    local exit_code_expectation_file="${6}"
    local stderr_expected="${7}"
    local stdout_expectation_file="${8}" # the file to write to when user chooses to update stdout expectation
    local stderr_expectation_file="${9}" # the file to write to when user chooses to update stderr expectation
    local stdout_path; stdout_path=$(mktemp)
    local stderr_path; stderr_path=$(mktemp)

    # shellcheck disable=SC2064
    trap "rm -f $stdout_path $stderr_path" EXIT

    if [[ "$exit_code_expected" = "" ]]; then exit_code_expected="0"; fi

    [[ $filename == "" ]] || solc_args+=("$filename")

    local solc_command="$SOLC ${solc_args[*]} <$solc_stdin"
    set +e
    "$SOLC" "${solc_args[@]}" <"$solc_stdin" >"$stdout_path" 2>"$stderr_path"
    exitCode=$?
    set -e

    if [[ " ${solc_args[*]} " == *" --standard-json "* ]]
    then
        sed -i.bak -e 's/{[^{]*Warning: This is a pre-release compiler version[^}]*},\{0,1\}//' "$stdout_path"
        sed -i.bak -E -e 's/ Consider adding \\"pragma solidity \^[0-9.]*;\\"//g' "$stdout_path"
        sed -i.bak -e 's/"errors":\[\],\{0,1\}//' "$stdout_path"
        sed -i.bak -E -e 's/\"opcodes\":\"[^"]+\"/\"opcodes\":\"<OPCODES REMOVED>\"/g' "$stdout_path"
        sed -i.bak -E -e 's/\"sourceMap\":\"[0-9:;-]+\"/\"sourceMap\":\"<SOURCEMAP REMOVED>\"/g' "$stdout_path"

        # Remove bytecode (but not linker references).
        sed -i.bak -E -e 's/(\"object\":\")[0-9a-f]+([^"]*\")/\1<BYTECODE REMOVED>\2/g' "$stdout_path"
        # shellcheck disable=SC2016
        sed -i.bak -E -e 's/(\"object\":\"[^"]+\$__)[0-9a-f]+(\")/\1<BYTECODE REMOVED>\2/g' "$stdout_path"
        # shellcheck disable=SC2016
        sed -i.bak -E -e 's/([0-9a-f]{34}\$__)[0-9a-f]+(__\$[0-9a-f]{17})/\1<BYTECODE REMOVED>\2/g' "$stdout_path"
        # Remove metadata in assembly output (see below about the magic numbers)
        sed -i.bak -E -e 's/"[0-9a-f]+64697066735822[0-9a-f]+64736f6c63[0-9a-f]+/"<BYTECODE REMOVED>/g' "$stdout_path"
        # Remove hash of text representation in ewasm
        sed -i.bak -E -e 's/The Keccak-256 hash of the text representation of .*: [0-9a-f]+/The Keccak-256 hash of the text representation of <REMOVED>/g' "$stdout_path"

        # Replace escaped newlines by actual newlines for readability
        # shellcheck disable=SC1003
        sed -i.bak -E -e 's/\\n/\'$'\n/g' "$stdout_path"
        rm "$stdout_path.bak"
    else
        sed -i.bak -e '/^Warning: This is a pre-release compiler version, please do not use it in production./d' "$stderr_path"
        sed -i.bak -e '/^Warning (3805): This is a pre-release compiler version, please do not use it in production./d' "$stderr_path"
        sed -i.bak -e 's/\(^[ ]*auxdata: \)0x[0-9a-f]*$/\1<AUXDATA REMOVED>/' "$stdout_path"
        sed -i.bak -e 's/ Consider adding "pragma .*$//' "$stderr_path"
        sed -i.bak -e 's/\(Unimplemented feature error.* in \).*$/\1<FILENAME REMOVED>/' "$stderr_path"
        sed -i.bak -e 's/"version":[ ]*"[^"]*"/"version": "<VERSION REMOVED>"/' "$stdout_path"

        # Remove bytecode (but not linker references). Since non-JSON output is unstructured,
        # use metadata markers for detection to have some confidence that it's actually bytecode
        # and not some random word.
        # 64697066735822 = hex encoding of 0x64 'i' 'p' 'f' 's' 0x58 0x22
        # 64736f6c63     = hex encoding of 0x64 's' 'o' 'l' 'c'
        sed -i.bak -E -e 's/[0-9a-f]*64697066735822[0-9a-f]+64736f6c63[0-9a-f]+/<BYTECODE REMOVED>/g' "$stdout_path"
        # shellcheck disable=SC2016
        sed -i.bak -E -e 's/([0-9a-f]{17}\$__)[0-9a-f]+(__\$[0-9a-f]{17})/\1<BYTECODE REMOVED>\2/g' "$stdout_path"
        # shellcheck disable=SC2016
        sed -i.bak -E -e 's/[0-9a-f]+((__\$[0-9a-f]{34}\$__)*<BYTECODE REMOVED>)/<BYTECODE REMOVED>\1/g' "$stdout_path"

        # Remove trailing empty lines. Needs a line break to make OSX sed happy.
        sed -i.bak -e '1{/^$/d
}' "$stderr_path"
        rm "$stderr_path.bak" "$stdout_path.bak"
    fi
    # Remove path to cpp file
    sed -i.bak -e 's/^\(Exception while assembling:\).*/\1/' "$stderr_path"
    # Remove exception class name.
    sed -i.bak -e 's/^\(Dynamic exception type:\).*/\1/' "$stderr_path"
    rm "$stderr_path.bak"

    if [[ $exitCode -ne "$exit_code_expected" ]]
    then
        printError "Incorrect exit code. Expected $exit_code_expected but got $exitCode."

        [[ $exit_code_expectation_file != "" ]] && ask_expectation_update "$exitCode" "$exit_code_expectation_file"
        [[ $exit_code_expectation_file == "" ]] && exit 1
    fi

    if [[ "$(cat "$stdout_path")" != "${stdout_expected}" ]]
    then
        printError "Incorrect output on stdout received. Expected:"
        echo -e "${stdout_expected}"

        printError "But got:"
        echo -e "$(cat "$stdout_path")"

        printError "When running $solc_command"

        [[ $stdout_expectation_file != "" ]] && ask_expectation_update "$(cat "$stdout_path")" "$stdout_expectation_file"
        [[ $stdout_expectation_file == "" ]] && exit 1
    fi

    if [[ "$(cat "$stderr_path")" != "${stderr_expected}" ]]
    then
        printError "Incorrect output on stderr received. Expected:"
        echo -e "${stderr_expected}"

        printError "But got:"
        echo -e "$(cat "$stderr_path")"

        printError "When running $solc_command"

        [[ $stderr_expectation_file != "" ]] && ask_expectation_update "$(cat "$stderr_path")" "$stderr_expectation_file"
        [[ $stderr_expectation_file == "" ]] && exit 1
    fi

    rm -f "$stdout_path" "$stderr_path"
}


function test_solc_assembly_output()
{
    local input="${1}"
    local expected="${2}"
    IFS=" " read -r -a solc_args <<< "${3}"

    local expected_object="object \"object\" { code ${expected} }"

    output=$(echo "${input}" | "$SOLC" - "${solc_args[@]}" 2>/dev/null)
    empty=$(echo "$output" | tr '\n' ' ' | tr -s ' ' | sed -ne "/${expected_object}/p")
    if [ -z "$empty" ]
    then
        printError "Incorrect assembly output. Expected: "
        echo -e "${expected}"
        printError "with arguments ${solc_args[*]}, but got:"
        echo "${output}"
        exit 1
    fi
}

## RUN

echo "Checking that the bug list is up to date..."
"$REPO_ROOT"/scripts/update_bugs_by_version.py

printTask "Testing unknown options..."
(
    set +e
    output=$("$SOLC" --allow=test 2>&1)
    failed=$?
    set -e

    if [ "$output" == "unrecognised option '--allow=test'" ] && [ $failed -ne 0 ]
    then
        echo "Passed"
    else
        printError "Incorrect response to unknown options: $output"
        exit 1
    fi
)


printTask "Testing passing files that are not found..."
test_solc_behaviour "file_not_found.sol" "" "" "" 1 "" "\"file_not_found.sol\" is not found." "" ""

printTask "Testing passing files that are not files..."
test_solc_behaviour "." "" "" "" 1 "" "\".\" is not a valid file." "" ""

printTask "Testing passing empty remappings..."
test_solc_behaviour "${0}" "=/some/remapping/target" "" "" 1 "" "Invalid remapping: \"=/some/remapping/target\"." "" ""
test_solc_behaviour "${0}" "ctx:=/some/remapping/target" "" "" 1 "" "Invalid remapping: \"ctx:=/some/remapping/target\"." "" ""

printTask "Running general commandline tests..."
(
    cd "$REPO_ROOT"/test/cmdlineTests/
    for tdir in */
    do
        printTask " - ${tdir}"

        # Strip trailing slash from $tdir.
        tdir=$(basename "${tdir}")

        inputFiles="$(ls -1 "${tdir}/input."* 2> /dev/null || true)"
        inputCount="$(echo "${inputFiles}" | wc -w)"
        if (( inputCount > 1 ))
        then
            printError "Ambiguous input. Found input files in multiple formats:"
            echo -e "${inputFiles}"
            exit 1
        fi

        # Use printf to get rid of the trailing newline
        inputFile=$(printf "%s" "${inputFiles}")

        if [ "${inputFile}" = "${tdir}/input.json" ]
        then
            ! [ -e "${tdir}/stdin" ] || { printError "Found a file called 'stdin' but redirecting standard input in JSON mode is not allowed."; exit 1; }

            stdin="${inputFile}"
            inputFile=""
            stdout="$(cat "${tdir}/output.json" 2>/dev/null || true)"
            stdoutExpectationFile="${tdir}/output.json"
            command_args="--standard-json "$(cat "${tdir}/args" 2>/dev/null || true)
        else
            if [ -e "${tdir}/stdin" ]
            then
                stdin="${tdir}/stdin"
                [ -f "${tdir}/stdin" ] || { printError "'stdin' is not a regular file."; exit 1; }
            else
                stdin=""
            fi

            stdout="$(cat "${tdir}/output" 2>/dev/null || true)"
            stdoutExpectationFile="${tdir}/output"
            command_args=$(cat "${tdir}/args" 2>/dev/null || true)
        fi
        exitCodeExpectationFile="${tdir}/exit"
        exitCode=$(cat "$exitCodeExpectationFile" 2>/dev/null || true)
        err="$(cat "${tdir}/err" 2>/dev/null || true)"
        stderrExpectationFile="${tdir}/err"
        test_solc_behaviour "$inputFile" \
                            "$command_args" \
                            "$stdin" \
                            "$stdout" \
                            "$exitCode" \
                            "$exitCodeExpectationFile" \
                            "$err" \
                            "$stdoutExpectationFile" \
                            "$stderrExpectationFile"
    done
)

printTask "Compiling various other contracts and libraries..."
(
    cd "$REPO_ROOT"/test/compilationTests/
    for dir in */
    do
        echo " - $dir"
        cd "$dir"
        compileFull -w ./*.sol ./*/*.sol
        cd ..
    done
)

printTask "Compiling all examples from the documentation..."
SOLTMPDIR=$(mktemp -d)
(
    set -e
    cd "$SOLTMPDIR"
    "$REPO_ROOT"/scripts/isolate_tests.py "$REPO_ROOT"/docs/ docs
    developmentVersion=$("$REPO_ROOT/scripts/get_version.sh")

    for f in *.sol
    do
        # The contributors guide uses syntax tests, but we cannot
        # really handle them here.
        if grep -E 'DeclarationError:|// ----' "$f" >/dev/null
        then
            continue
        fi
        echo "$f"

        opts=()
        # We expect errors if explicitly stated, or if imports
        # are used (in the style guide)
        if grep -E "This will not compile|import \"" "$f" >/dev/null
        then
            opts=(-e)
        fi
        if grep "This will report a warning" "$f" >/dev/null
        then
            opts+=(-w)
        fi
        if grep "This may report a warning" "$f" >/dev/null
        then
            opts+=(-o)
        fi

        # Disable the version pragma in code snippets that only work with the current development version.
        # It's necessary because x.y.z won't match `^x.y.z` or `>=x.y.z` pragmas until it's officially released.
        sed -i.bak -E -e 's/pragma[[:space:]]+solidity[[:space:]]*(\^|>=)[[:space:]]*'"$developmentVersion"'/pragma solidity >0.0.1/' "$f"
        compileFull "${opts[@]}" "$SOLTMPDIR/$f"
    done
)
rm -rf "$SOLTMPDIR"
echo "Done."

printTask "Testing library checksum..."
echo '' | "$SOLC" - --link --libraries a=0x90f20564390eAe531E810af625A22f51385Cd222 >/dev/null
echo '' | "$SOLC" - --link --libraries a=0x80f20564390eAe531E810af625A22f51385Cd222 &>/dev/null && exit 1

printTask "Testing long library names..."
echo '' | "$SOLC" - --link --libraries aveeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeerylonglibraryname=0x90f20564390eAe531E810af625A22f51385Cd222 >/dev/null

printTask "Testing linking itself..."
SOLTMPDIR=$(mktemp -d)
(
    cd "$SOLTMPDIR"
    set -e
    echo 'library L { function f() public pure {} } contract C { function f() public pure { L.f(); } }' > x.sol
    "$SOLC" --bin -o . x.sol 2>/dev/null
    # Explanation and placeholder should be there
    grep -q '//' C.bin && grep -q '__' C.bin
    # But not in library file.
    grep -q -v '[/_]' L.bin
    # Now link
    "$SOLC" --link --libraries x.sol:L=0x90f20564390eAe531E810af625A22f51385Cd222 C.bin
    # Now the placeholder and explanation should be gone.
    grep -q -v '[/_]' C.bin
)
rm -rf "$SOLTMPDIR"

printTask "Testing overwriting files..."
SOLTMPDIR=$(mktemp -d)
(
    set -e
    # First time it works
    echo 'contract C {} ' | "$SOLC" - --bin -o "$SOLTMPDIR/non-existing-stuff-to-create" 2>/dev/null
    # Second time it fails
    echo 'contract C {} ' | "$SOLC" - --bin -o "$SOLTMPDIR/non-existing-stuff-to-create" 2>/dev/null && exit 1
    # Unless we force
    echo 'contract C {} ' | "$SOLC" - --overwrite --bin -o "$SOLTMPDIR/non-existing-stuff-to-create" 2>/dev/null
)
rm -rf "$SOLTMPDIR"

printTask "Testing assemble, yul, strict-assembly and optimize..."
(
    echo '{}' | "$SOLC" - --assemble &>/dev/null
    echo '{}' | "$SOLC" - --yul &>/dev/null
    echo '{}' | "$SOLC" - --strict-assembly &>/dev/null

    # Test options above in conjunction with --optimize.
    # Using both, --assemble and --optimize should fail.
    echo '{}' | "$SOLC" - --assemble --optimize &>/dev/null && exit 1
    echo '{}' | "$SOLC" - --yul --optimize &>/dev/null && exit 1

    # Test yul and strict assembly output
    # Non-empty code results in non-empty binary representation with optimizations turned off,
    # while it results in empty binary representation with optimizations turned on.
    test_solc_assembly_output "{ let x:u256 := 0:u256 }" "{ let x := 0 }" "--yul"
    test_solc_assembly_output "{ let x:u256 := bitnot(7:u256) }" "{ let x := bitnot(7) }" "--yul"
    test_solc_assembly_output "{ let t:bool := not(true) }" "{ let t:bool := not(true) }" "--yul"
    test_solc_assembly_output "{ let x := 0 }" "{ let x := 0 }" "--strict-assembly"
    test_solc_assembly_output "{ let x := 0 }" "{ { } }" "--strict-assembly --optimize"
)


printTask "Testing standard input..."
SOLTMPDIR=$(mktemp -d)
(
    set +e
    output=$("$SOLC" --bin  2>&1)
    result=$?
    set -e

    # This should fail
    if [[ ! ("$output" =~ "No input files given") || ($result == 0) ]]
    then
        printError "Incorrect response to empty input arg list: $output"
        exit 1
    fi

    # The contract should be compiled
    if ! output=$(echo 'contract C {} ' | "$SOLC" - --bin 2>/dev/null | grep -q "<stdin>:C")
    then
        printError "Failed to compile a simple contract from standard input"
        exit 1
    fi

    # This should not fail
    if ! output=$(echo '' | "$SOLC" --ast-compact-json - 2>/dev/null)
    then
        printError "Incorrect response to --ast-compact-json option with empty stdin"
        exit 1
    fi
)

printTask "Testing AST import..."
SOLTMPDIR=$(mktemp -d)
(
    cd "$SOLTMPDIR"
    if ! "$REPO_ROOT/scripts/ASTImportTest.sh"
    then
        rm -rf "$SOLTMPDIR"
        exit 1
    fi
)
rm -rf "$SOLTMPDIR"

printTask "Testing AST export with stop-after=parsing..."
"$REPO_ROOT/test/stopAfterParseTests.sh"

printTask "Testing soljson via the fuzzer..."
SOLTMPDIR=$(mktemp -d)
(
    set -e
    cd "$SOLTMPDIR"
    "$REPO_ROOT"/scripts/isolate_tests.py "$REPO_ROOT"/test/
    "$REPO_ROOT"/scripts/isolate_tests.py "$REPO_ROOT"/docs/ docs

    echo ./*.sol | xargs -P 4 -n 50 "${SOLIDITY_BUILD_DIR}/test/tools/solfuzzer" --quiet --input-files
    echo ./*.sol | xargs -P 4 -n 50 "${SOLIDITY_BUILD_DIR}/test/tools/solfuzzer" --without-optimizer --quiet --input-files
)
rm -rf "$SOLTMPDIR"

echo "Commandline tests successful."
