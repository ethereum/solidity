#!/usr/bin/env bash
#------------------------------------------------------------------------------
# Bash script to execute the Solidity tests by CircleCI.
#
# The documentation for solidity is hosted at:
#
#     https://docs.soliditylang.org
#
# ------------------------------------------------------------------------------
# Configuration Environment Variables:
#
#     EVM=version_string      Specifies EVM version to compile for (such as homestead, etc)
#     OPTIMIZE=1              Enables backend optimizer
#     ABI_ENCODER_V1=1        Forcibly enables ABI coder version 1
#     SOLTEST_FLAGS=<flags>   Appends <flags> to default SOLTEST_ARGS
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
# (c) 2016-2019 solidity contributors.
# ------------------------------------------------------------------------------
set -e

OPTIMIZE=${OPTIMIZE:-"0"}
EVM=${EVM:-"invalid"}
REPODIR="$(realpath "$(dirname "$0")/..")"

IFS=" " read -r -a BOOST_TEST_ARGS <<< "$BOOST_TEST_ARGS"
IFS=" " read -r -a SOLTEST_FLAGS <<< "$SOLTEST_FLAGS"

# shellcheck source=scripts/common.sh
source "${REPODIR}/scripts/common.sh"
# Test result output directory (CircleCI is reading test results from here)
mkdir -p test_results

# in case we run with ASAN enabled, we must increase stck size.
ulimit -s 16384

get_logfile_basename() {
    local filename="${EVM}"
    test "${OPTIMIZE}" = "1" && filename="${filename}_opt"
    test "${ABI_ENCODER_V1}" = "1" && filename="${filename}_abiv1"

    echo -ne "${filename}"
}

# FIXME: Test results from each run must be joined into a single file
BOOST_TEST_ARGS=("--color_output=no" "--show_progress=no" "--logger=JUNIT,error,test_results/$(get_logfile_basename).xml" "${BOOST_TEST_ARGS[@]}")
SOLTEST_ARGS=("--evm-version=$EVM" "${SOLTEST_FLAGS[@]}")

test "${OPTIMIZE}" = "1" && SOLTEST_ARGS+=(--optimize)
test "${ABI_ENCODER_V1}" = "1" && SOLTEST_ARGS+=(--abiencoderv1)

all_test_cases=$("${REPODIR}/build/test/soltest" --list_content 2>&1)

batch_count=9
pids=()
for batch in $(seq "$batch_count")
do
    selected_test_cases=$(echo "$all_test_cases" | python3 "${REPODIR}/scripts/isoltest_test_case_names_from_list_content.py" "$batch" "$batch_count")
    # TODO: Capture output and print it when the process ends
    "${REPODIR}/build/test/soltest" "${BOOST_TEST_ARGS[@]}" --run_test="$selected_test_cases" -- "${SOLTEST_ARGS[@]}" & pids+=($!)
    echo "Running soltest ${BOOST_TEST_ARGS[*]} --run_test=<batch #${batch}> -- ${SOLTEST_ARGS[*]} (PID=${pids[-1]})"
done

for pid in "${pids[@]}"
do
    wait "$pid"
    echo "Process ${pid} finished.";
done
