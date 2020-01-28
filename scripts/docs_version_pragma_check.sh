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

set -e

## GLOBAL VARIABLES

REPO_ROOT=$(cd $(dirname "$0")/.. && pwd)
SOLIDITY_BUILD_DIR=${SOLIDITY_BUILD_DIR:-build}
source "${REPO_ROOT}/scripts/common.sh"
source "${REPO_ROOT}/scripts/common_cmdline.sh"

printTask "Compiling all examples from the documentation..."
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
        if grep -E "This will not compile|import \"" "$f" >/dev/null
        then
            opts="-e"
        fi
        if grep -E "This will report a warning|pragma experimental SMTChecker" "$f" >/dev/null
        then
            opts="$opts -w"
        fi

        grep -Po "/pragma/folk" "$f"

        if grep -E "(pragma solidity >=)" "$f"
        then
            echo "pragma solidity >="
        fi

        if grep -E "pragma solidity >" "$f"
        then
            echo "pragma solidity >"
        fi

        # Get minimum compiler version defined by pragma
        version="0.6.2"
        solc_bin="solc-$version"
        if [[ ! -f "$solc_bin" ]]; then
            # wget https://github.com/ethereum/solidity/releases/download/v$version/solc-static-linux
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