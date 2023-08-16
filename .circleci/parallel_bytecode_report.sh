#!/usr/bin/env bash
set -euo pipefail

#------------------------------------------------------------------------------
# Splits all test source code into multiple files, generates bytecode and metadata
# for each file and combines it into a single report.txt file.
#
# The script is meant to be executed in CI on all supported platforms. All generated
# reports must be identical for a given compiler version.
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
# (c) 2023 solidity contributors.
#------------------------------------------------------------------------------

(( $# == 4 )) || { >&2 echo "Wrong number of arguments."; exit 1; }
label="$1"
binary_type="$2"
binary_path="$3" # This path must be absolute
preset="$4"

[[ $binary_type == native || $binary_type == solcjs ]] || { >&2 echo "Invalid binary type: ${binary_type}"; exit 1; }

# NOTE: Locale affects the order of the globbed files.
export LC_ALL=C

mkdir test-cases/
cd test-cases/

echo "Preparing input files"
python3 ../scripts/isolate_tests.py ../test/

# FIXME: These cases crash because of https://github.com/ethereum/solidity/issues/13583
rm ./*_bytecode_too_large_*.sol ./*_combined_too_large_*.sol

if [[ $binary_type == native ]]; then
    interface=$(echo -e "standard-json\ncli" | circleci tests split)
    echo "Selected interface: ${interface}"

    echo "Generating bytecode reports"
    python3 ../scripts/bytecodecompare/prepare_report.py \
        "$binary_path" \
        --interface "$interface" \
        --preset "$preset" \
        --report-file "../bytecode-report-${label}-${interface}-${preset}.txt"
else
    echo "Installing solc-js"
    git clone --depth 1 https://github.com/ethereum/solc-js.git solc-js
    cp "$binary_path" solc-js/soljson.js

    cd solc-js/
    npm install
    npm run build

    cd ..
    npm install ./solc-js/dist

    cp ../scripts/bytecodecompare/prepare_report.js .

    echo "Generating bytecode reports"
    # shellcheck disable=SC2035
    ./prepare_report.js \
        --preset "$preset" \
        *.sol > "../bytecode-report-${label}-${preset}.txt"
fi
