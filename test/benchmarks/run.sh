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

result_legacy_file=$(mktemp -t benchmark-legacy-XXXXXX.txt)
result_via_ir_file=$(mktemp -t benchmark-via-ir-XXXXXX.txt)

function cleanup() {
  rm "${result_legacy_file}"
  rm "${result_via_ir_file}"
  exit
}

trap cleanup SIGINT SIGTERM

solc="${SOLIDITY_BUILD_DIR}/solc/solc"
benchmarks_dir="${REPO_ROOT}/test/benchmarks"
time_bin_path=$(type -P time)

for input_file in "chains.sol" "OptimizorClub.sol"
do
  input_path="${benchmarks_dir}/${input_file}"

  solc_command_legacy=("${solc}" --optimize --bin "${input_path}")
  solc_command_via_ir=("${solc}" --via-ir --optimize --bin "${input_path}")

  "${time_bin_path}" --output "${result_legacy_file}" --format "%e" "${solc_command_legacy[@]}" >/dev/null
  "${time_bin_path}" --output "${result_via_ir_file}" --format "%e" "${solc_command_via_ir[@]}" >/dev/null

  time_legacy=$(<"${result_legacy_file}")
  time_via_ir=$(<"${result_via_ir_file}")

  echo "======================================================="
  echo "            ${input_file}"
  echo "======================================================="
  echo "legacy pipeline took ${time_legacy} seconds to execute."
  echo "via-ir pipeline took ${time_via_ir} seconds to execute."
  echo "======================================================="
done

cleanup

