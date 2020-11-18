#!/usr/bin/env bash

#------------------------------------------------------------------------------
# Prints the exact version string that would be used to describe a nightly
# build of the compiler.
#
# The documentation for solidity is hosted at:
#
#     https://docs.soliditylang.org
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
# (c) 2017 solidity contributors.
#------------------------------------------------------------------------------

set -e

script_dir="$(dirname "$0")"

solidity_version=$("${script_dir}/get_version.sh")
last_commit_timestamp=$(git log -1 --date=iso --format=%ad HEAD)
last_commit_date=$(date --date="$last_commit_timestamp" --utc +%Y.%-m.%-d)
last_commit_hash=$(git rev-parse --short=8 HEAD)

echo "v${solidity_version}-nightly.${last_commit_date}+commit.${last_commit_hash}"
