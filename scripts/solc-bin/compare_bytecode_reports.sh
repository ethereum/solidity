#!/usr/bin/env bash

# ------------------------------------------------------------------------------
# Compares bytecode reports from multiple compiler versions and platforms to
# determine if there are any differences.
#
# The script does not accept any arguments.
#
# The reports should be placed in subdirectories of the current working
# directory and their names should follow one of the following patterns:
#
#     report-solc-<platform>-v<solidity version>+commit.<commit hash>.txt
#     report-soljson-<platform>-v<solidity version>+commit.<commit hash>.js.txt
#
# Reports corresponding to the same version and commit hash are grouped together
# and the script succeeds only if the files within each group are identical.
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
# (c) 2020 solidity contributors.
#------------------------------------------------------------------------------

set -euo pipefail

report_files="$(find . -type f -name 'report-*.txt')"
[[ $report_files != "" ]] || { echo "No reports found in the working directory."; exit 0; }

echo "Available reports:"
echo "$report_files"

versions_in_report_names=$(
    echo "$report_files" |
    sed -n -E 's/^\.\/[^\/]+\/report-(solc|soljson)-[0-9a-zA-Z-]+-v([0-9.]+\+commit\.[0-9a-f]+)(.[^.]+)?\.txt$/\2/p' |
    sort -V |
    uniq
)

num_failed_comparisons=0
for solidity_version_and_commit in $versions_in_report_names; do
    echo "Comparing reports for Solidity ${solidity_version_and_commit}:"
    mapfile -t report_files_for_version < <(
        echo "$report_files" |
        sed -n -E '/^\.\/[^\/]+\/report-(solc|soljson)-[0-9a-zA-Z-]+-v'"${solidity_version_and_commit//\+/\\+}"'+(.[^.]+)?\.txt$/p'
    )

    diff --report-identical-files --brief --from-file "${report_files_for_version[@]}" || ((++num_failed_comparisons))
    echo
done

(( num_failed_comparisons == 0 )) || { echo "Found bytecode differences in ${num_failed_comparisons} versions"; exit 1; }
echo "No bytecode differences found."
