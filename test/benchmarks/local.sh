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

(( $# <= 1 )) || fail "Too many arguments. Usage: local.sh [<solc-path>]"

solc="${1:-${SOLIDITY_BUILD_DIR}/solc/solc}"
command_available "$solc" --version
command_available "$(type -P time)" --version

output_dir=$(mktemp -d -t solc-benchmark-XXXXXX)

function cleanup() {
    rm -r "${output_dir}"
    exit
}

trap cleanup SIGINT SIGTERM

function benchmark_contract {
    local pipeline="$1"
    local input_path="$2"

    local solc_command=("${solc}" --optimize --bin --color "${input_path}")
    [[ $pipeline == ir ]] && solc_command+=(--via-ir)
    local time_file="${output_dir}/time-and-status-${pipeline}.txt"

    # NOTE: Legacy pipeline may fail with "Stack too deep" in some cases. That's fine.
    gnu_time_to_json_file "$time_file" \
        "${solc_command[@]}" \
        > "${output_dir}/bytecode-${pipeline}.bin" \
        2>> "${output_dir}/benchmark-warn-err.txt" || [[ $pipeline == legacy ]]

    printf '| %-20s | %8s | %7d bytes | %6.2f s | %9d MiB | %9d |\n' \
        '`'"$input_file"'`' \
        "$pipeline" \
        "$(bytecode_size < "${output_dir}/bytecode-${pipeline}.bin")" \
        "$(jq '(.user + .sys) * 100 | round / 100' "$time_file")" \
        "$(jq '.mem / 1024 | round' "$time_file")" \
        "$(jq '.exit' "$time_file")"
}

benchmarks=("verifier.sol" "OptimizorClub.sol" "chains.sol")

echo "|         File         | Pipeline | Bytecode size |   Time   | Memory (peak) | Exit code |"
echo "|----------------------|----------|--------------:|---------:|--------------:|----------:|"

for input_file in "${benchmarks[@]}"
do
    benchmark_contract legacy "${REPO_ROOT}/test/benchmarks/${input_file}"
    benchmark_contract ir     "${REPO_ROOT}/test/benchmarks/${input_file}"
done

echo
echo "======================================================="
echo "Warnings and errors generated during run:"
echo "======================================================="
echo "$(< "${output_dir}/benchmark-warn-err.txt")"

cleanup
