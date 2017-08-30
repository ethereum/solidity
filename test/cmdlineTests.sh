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

echo "Checking that the bug list is up to date..."
"$REPO_ROOT"/scripts/update_bugs_by_version.py

echo "Checking that StandardToken.sol, owned.sol and mortal.sol produce bytecode..."
output=$("$REPO_ROOT"/build/solc/solc --bin "$REPO_ROOT"/std/*.sol 2>/dev/null | grep "ffff" | wc -l)
test "${output//[[:blank:]]/}" = "3"

function compileFull()
{
    files="$*"
    set +e
    "$SOLC" --optimize \
    --combined-json abi,asm,ast,bin,bin-runtime,clone-bin,compact-format,devdoc,hashes,interface,metadata,opcodes,srcmap,srcmap-runtime,userdoc \
    $files >/dev/null 2>&1
    failed=$?
    set -e
    if [ $failed -ne 0 ]
    then
        echo "Compilation failed on:"
        cat $files
        false
    fi
}

function compileWithoutWarning()
{
    files="$*"
    set +e
    output=$("$SOLC" $files 2>&1)
    failed=$?
    # Remove the pre-release warning from the compiler output
    output=$(echo "$output" | grep -v 'pre-release')
    echo "$output"
    set -e
    test -z "$output" -a "$failed" -eq 0
}

echo "Compiling various other contracts and libraries..."
(
cd "$REPO_ROOT"/test/compilationTests/
for dir in *
do
    if [ "$dir" != "README.md" ]
    then
        echo " - $dir"
        cd "$dir"
        compileFull *.sol */*.sol
        cd ..
    fi
done
)

echo "Compiling all files in std and examples..."

for f in "$REPO_ROOT"/std/*.sol
do
    echo "$f"
    compileWithoutWarning "$f"
done

echo "Compiling all examples from the documentation..."
TMPDIR=$(mktemp -d)
(
    set -e
    cd "$REPO_ROOT"
    REPO_ROOT=$(pwd) # make it absolute
    cd "$TMPDIR"
    "$REPO_ROOT"/scripts/isolate_tests.py "$REPO_ROOT"/docs/ docs
    for f in *.sol
    do
        echo "$f"
        compileFull "$TMPDIR/$f"
    done
)
rm -rf "$TMPDIR"
echo "Done."

echo "Testing library checksum..."
echo '' | "$SOLC" --link --libraries a:0x90f20564390eAe531E810af625A22f51385Cd222
! echo '' | "$SOLC" --link --libraries a:0x80f20564390eAe531E810af625A22f51385Cd222 2>/dev/null

echo "Testing long library names..."
echo '' | "$SOLC" --link --libraries aveeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeerylonglibraryname:0x90f20564390eAe531E810af625A22f51385Cd222

echo "Testing overwriting files"
TMPDIR=$(mktemp -d)
(
    set -e
    # First time it works
    echo 'contract C {} ' | "$SOLC" --bin -o "$TMPDIR/non-existing-stuff-to-create" 2>/dev/null
    # Second time it fails
    ! echo 'contract C {} ' | "$SOLC" --bin -o "$TMPDIR/non-existing-stuff-to-create" 2>/dev/null
    # Unless we force
    echo 'contract C {} ' | "$SOLC" --overwrite --bin -o "$TMPDIR/non-existing-stuff-to-create" 2>/dev/null
)
rm -rf "$TMPDIR"

echo "Testing soljson via the fuzzer..."
TMPDIR=$(mktemp -d)
(
    set -e
    cd "$REPO_ROOT"
    REPO_ROOT=$(pwd) # make it absolute
    cd "$TMPDIR"
    "$REPO_ROOT"/scripts/isolate_tests.py "$REPO_ROOT"/test/
    "$REPO_ROOT"/scripts/isolate_tests.py "$REPO_ROOT"/docs/ docs
    for f in *.sol
    do
        set +e
        "$REPO_ROOT"/build/test/solfuzzer --quiet < "$f"
        if [ $? -ne 0 ]; then
            echo "Fuzzer failed on:"
            cat "$f"
            exit 1
        fi

        "$REPO_ROOT"/build/test/solfuzzer --without-optimizer --quiet < "$f"
        if [ $? -ne 0 ]; then
            echo "Fuzzer (without optimizer) failed on:"
            cat "$f"
            exit 1
        fi
        set -e
    done
)
rm -rf "$TMPDIR"
echo "Done."
