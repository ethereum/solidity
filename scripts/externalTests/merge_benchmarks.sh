#!/usr/bin/env bash

# ------------------------------------------------------------------------------
# Reads multiple individual benchmark reports produced by scripts from
# test/externalTests/ from standard input and creates a combined report.
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

# We expect a series of dicts of the form {"<project>": {"<preset>": {...}}}.
# Unfortunately jq's built-in `add` filter can't handle nested dicts and
# would just overwrite values sharing a project name instead of merging them.

# This is done by first grouping the dicts into an array of the form
# [
#     [{"key": "<project1>", "value": {"<preset1>": {...}}}, {"key": "<project1>", "value": {"<preset2>": {...}}, ...],
#     [{"key": "<project2>", "value": {"<preset1>": {...}}}, {"key": "<project2>", "value": {"<preset2>": {...}}, ...],
#     ...
# ]
# and then using reduce() on each group sharing the same project name to convert it into a
# dict having preset names as keys.
jq --slurp  --indent 4 --sort-keys '
    map(to_entries[]) |
    group_by(.key) |
    map({
        (.[0].key): (
            reduce (.[].value | to_entries[]) as {$key, $value} (
                {}; . + {
                    ($key): $value
                }
            )
        )
    }) |
    add
'
