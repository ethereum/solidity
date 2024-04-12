#!/usr/bin/env bash

#------------------------------------------------------------------------------
# Bash script to run optimizer performance tests.
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
# (c) 2022 solidity contributors.
#------------------------------------------------------------------------------

set -euo pipefail

REPO_ROOT=$(cd "$(dirname "$0")/../../" && pwd)
SOLIDITY_BUILD_DIR=${SOLIDITY_BUILD_DIR:-${REPO_ROOT}/build}

# shellcheck source=scripts/common.sh
source "${REPO_ROOT}/scripts/common.sh"
# shellcheck source=scripts/common_cmdline.sh
source "${REPO_ROOT}/scripts/common_cmdline.sh"

(( $# <= 1 )) || fail "Too many arguments. Usage: run.sh [<solc-path>]"

solc="${1:-${SOLIDITY_BUILD_DIR}/solc/solc}"
command_available "$solc" --version

output_dir=$(mktemp -d -t solc-benchmark-XXXXXX)
result_legacy_file="${output_dir}/benchmark-legacy.txt"
result_via_ir_file="${output_dir}/benchmark-via-ir.txt"
warnings_and_errors_file="${output_dir}/benchmark-warn-err.txt"

function cleanup() {
    rm -r "${output_dir}"
    exit
}

trap cleanup SIGINT SIGTERM

function bytecode_size {
    local bytecode_chars
    bytecode_chars=$(stripCLIDecorations | stripEmptyLines | wc --chars)
    echo $(( bytecode_chars / 2 ))
}

benchmarks_dir="${REPO_ROOT}/test/benchmarks"
benchmarks=("chains.sol" "OptimizorClub.sol" "verifier.sol")
time_bin_path=$(type -P time)

echo "| File                 | Pipeline | Bytecode size | Time     | Exit code |"
echo "|----------------------|----------|--------------:|---------:|----------:|"

for input_file in "${benchmarks[@]}"
do
    input_path="${benchmarks_dir}/${input_file}"

    solc_command_legacy=("${solc}" --optimize --bin --color "${input_path}")
    solc_command_via_ir=("${solc}" --via-ir --optimize --bin --color "${input_path}")

    # Legacy can fail.
    "${time_bin_path}" --output "${result_legacy_file}" --quiet --format "%e s |         %x" "${solc_command_legacy[@]}" >"${output_dir}/bytecode-legacy.bin" 2>>"${warnings_and_errors_file}" || true
    "${time_bin_path}" --output "${result_via_ir_file}" --quiet --format "%e s |         %x" "${solc_command_via_ir[@]}" >"${output_dir}/bytecode-via-ir.bin" 2>>"${warnings_and_errors_file}"

    time_and_status_legacy=$(<"${result_legacy_file}")
    time_and_status_via_ir=$(<"${result_via_ir_file}")
    bytecode_size_legacy=$(bytecode_size <"${output_dir}/bytecode-legacy.bin")
    bytecode_size_via_ir=$(bytecode_size <"${output_dir}/bytecode-via-ir.bin")

    printf '| %-20s | legacy   | %7d bytes | %20s |\n' '`'"$input_file"'`' "$bytecode_size_legacy" "$time_and_status_legacy"
    printf '| %-20s | via-ir   | %7d bytes | %20s |\n' '`'"$input_file"'`' "$bytecode_size_via_ir" "$time_and_status_via_ir"
done

echo
echo "======================================================="
echo "Warnings and errors generated during run:"
echo "======================================================="
echo "$(<"${warnings_and_errors_file}")"

cleanup

