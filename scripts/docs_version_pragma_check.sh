#!/usr/bin/env bash

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

# This script verifies that the examples compile with the oldest version mentioned in the pragma.
# It does not verify that it cannot be compiled with an older version
# and it also does not verify that it can be compiled with the newest version compatible with the pragma.

set -e

## GLOBAL VARIABLES

REPO_ROOT=$(cd $(dirname "$0")/.. && pwd)
SOLIDITY_BUILD_DIR=${SOLIDITY_BUILD_DIR:-${REPO_ROOT}/build}
source "${REPO_ROOT}/scripts/common.sh"
source "${REPO_ROOT}/scripts/common_cmdline.sh"

function versionGreater()
{
    v1=$1
    v2=$2
    ver1=( ${v1//./ } )
    ver2=( ${v2//./ } )

    if (( ${ver1[0]} > ${ver2[0]} ))
    then
        return 0
    elif (( ${ver1[0]} == ${ver2[0]} )) && (( ${ver1[1]} > ${ver2[1]} ))
    then
        return 0
    elif (( ${ver1[0]} == ${ver2[0]} )) && (( ${ver1[1]} == ${ver2[1]} )) && (( ${ver1[2]} > ${ver2[2]} ))
    then
        return 0
    fi
    return 1
}

function versionEqual()
{
    if [ "$1" == "$2" ]
    then
        return 0
    fi
    return 1
}

function getAllAvailableVersions()
{
    allVersions=()
    local allListedVersions=( $(
        wget -q -O- https://binaries.soliditylang.org/bin/list.txt |
        grep -Po '(?<=soljson-v)\d+.\d+.\d+(?=\+commit)' |
        sort -V
    ) )
    for listed in "${allListedVersions[@]}"
    do
        if versionGreater "$listed" "0.4.10"
        then
            allVersions+=( $listed )
        fi
    done
}

function findMinimalVersion()
{
    local f=$1
    local greater=false
    local pragmaVersion

    # Get minimum compiler version defined by pragma
    if (grep -Po '(?<=pragma solidity >=)\d+.\d+.\d+' "$f" >/dev/null)
    then
        pragmaVersion="$(grep -Po '(?<=pragma solidity >=)\d+.\d+.\d+' "$f")"
        sign=">="
    elif (grep -Po '(?<=pragma solidity \^)\d+.\d+.\d+' "$f" >/dev/null)
    then
        pragmaVersion="$(grep -Po '(?<=pragma solidity \^)\d+.\d+.\d+' "$f")"
        sign="^"
    elif (grep -Po '(?<=pragma solidity >)\d+.\d+.\d+' "$f" >/dev/null)
    then
        pragmaVersion="$(grep -Po '(?<=pragma solidity >)\d+.\d+.\d+' "$f")"
        sign=">"
        greater=true;
    else
        printError "No valid pragma statement in file. Skipping..."
        return
    fi

    version=""
    for ver in "${allVersions[@]}"
    do
        if versionGreater "$ver" "$pragmaVersion"
        then
            version="$ver"
            break
        elif ([ $greater == false ]) && versionEqual "$ver" "$pragmaVersion"
        then
            version="$ver"
            break
        fi
    done

    if [ -z "$version" ]
    then
        if [[ "$greater" = true && "$pragmaVersion" =~ 99 ]]
        then
            printError "Skipping version check for pragma: $pragmaVersion"
        else
            printError "No release $sign$pragmaVersion was listed in available releases!"
            exit 1
        fi
    fi
}

printTask "Verifying that all examples from the documentation have the correct version range..."
SOLTMPDIR=$(mktemp -d)
(
    set -e
    cd "$SOLTMPDIR"
    "$REPO_ROOT"/scripts/isolate_tests.py "$REPO_ROOT"/docs/ docs

    getAllAvailableVersions

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
        if ( ! grep -E "This will not compile after" "$f" >/dev/null && \
            grep -E "This will not compile|import \"" "$f" >/dev/null )
        then
            opts="-e"
        fi

        # ignore warnings in this case
        opts="$opts -o"

        findMinimalVersion $f
        if [ -z "$version" ]
        then
            continue
        fi

        opts="$opts -v $version"

        solc_bin="solc-$version"
        echo "$solc_bin"
        if [[ ! -f "$solc_bin" ]]
        then
            echo "Downloading release from github..."
            if wget -q https://github.com/ethereum/solidity/releases/download/v$version/solc-static-linux >/dev/null
            then
                mv solc-static-linux $solc_bin
            else
                printError "No release $version was found on github!"
                continue
            fi
        fi

        ln -sf "$solc_bin" "solc"
        chmod a+x solc

        SOLC="$SOLTMPDIR/solc"
        compileFull $opts "$SOLTMPDIR/$f"
    done
)
rm -rf "$SOLTMPDIR"
echo "Done."
