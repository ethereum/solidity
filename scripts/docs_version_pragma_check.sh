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
SOLIDITY_BUILD_DIR=${SOLIDITY_BUILD_DIR:-build}
source "${REPO_ROOT}/scripts/common.sh"
source "${REPO_ROOT}/scripts/common_cmdline.sh"

printTask "Verifying that all examples from the documentation have the correct version range..."
SOLTMPDIR=$(mktemp -d)
(
    set -e
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
        if ( ! grep -E "This will not compile after" "$f" >/dev/null && \
            grep -E "This will not compile|import \"" "$f" >/dev/null )
        then
            opts="-e"
        fi

        # ignore warnings in this case
        opts="$opts -o"

        # Get minimum compiler version defined by pragma
        if (grep -Po '(?<=pragma solidity >=)\d+.\d+.\d+' "$f" >/dev/null); then
            version="$(grep -Po '(?<=pragma solidity >=)\d+.\d+.\d+' "$f")"
            if (echo $version | grep -Po '(?<=0.4.)\d+' >/dev/null); then
                patch=$(echo $version | grep -Po '(?<=0.4.)\d+')
                if (( patch < 11 )); then
                    version="0.4.11" # first available release on github
                fi
            fi
        elif (grep -Po '(?<=pragma solidity \^)\d+.\d+.\d+' "$f" >/dev/null); then
            version="$(grep -Po '(?<=pragma solidity \^)\d+.\d+.\d+' "$f")"
        fi

        opts="$opts -v $version"

        solc_bin="solc-$version"
        echo "$solc_bin"
        if [[ ! -f "$solc_bin" ]]; then
            echo "Downloading release from github..."
            wget https://github.com/ethereum/solidity/releases/download/v$version/solc-static-linux
            mv solc-static-linux $solc_bin
        fi

        ln -sf "$solc_bin" "solc"
        chmod a+x solc

        SOLC="$SOLTMPDIR/solc"
        compileFull $opts "$SOLTMPDIR/$f"
    done
)
rm -rf "$SOLTMPDIR"
echo "Done."