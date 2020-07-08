#!/usr/bin/env bash

#------------------------------------------------------------------------------
# Prints the exact version string that would be used to describe a nightly
# build of the compiler.
#
# The documentation for solidity is hosted at:
#
#     https://solidity.readthedocs.org
#
# ------------------------------------------------------------------------------
# SPDX-License-Identifier: GPL-3.0
#------------------------------------------------------------------------------

set -e

script_dir="$(dirname "$0")"

solidity_version=$("${script_dir}/get_version.sh")
last_commit_timestamp=$(git log -1 --date=iso --format=%ad HEAD)
last_commit_date=$(date --date="$last_commit_timestamp" --utc +%Y.%-m.%-d)
last_commit_hash=$(git rev-parse --short=8 HEAD)

echo "v${solidity_version}-nightly.${last_commit_date}+commit.${last_commit_hash}"
