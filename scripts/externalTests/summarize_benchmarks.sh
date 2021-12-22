#!/usr/bin/env bash

# ------------------------------------------------------------------------------
# Reads a combined benchmark report from standard input and outputs an abbreviated
# report containing only totals. Can handle individual reports coming directly
# from scripts in test/externalTests/ as well as combined report from merge_benchmarks.sh.
#
# Usage:
#    <script name>.sh < <CONCATENATED_REPORTS>
#
# CONCATENATED_REPORTS: JSON report files concatenated into a single stream (e.g. using cat).
#
# Example:
#    cat reports/externalTests/benchmark-*.json | <script name>.sh
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
# (c) 2021 solidity contributors.
#------------------------------------------------------------------------------

set -euo pipefail

REPO_ROOT=$(realpath "$(dirname "$0")/../..")

# Iterates over presets in a dict of the form {"<project>": {"<preset>": {...}}} and for each
# one preserves only the few keys with totals that we want to see in the summary.
exec "${REPO_ROOT}/scripts/externalTests/merge_benchmarks.sh" | jq --indent 4 --sort-keys '
    with_entries({
        key: .key,
        value: .value | with_entries({
            key: .key,
            value: {
                bytecode_size: .value.total_bytecode_size,
                method_gas: .value.gas.total_method_gas,
                deployment_gas: .value.gas.total_deployment_gas,
                version: .value.project.version
            }
        })
    })
'
