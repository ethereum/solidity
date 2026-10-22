#!/usr/bin/env bash

# ------------------------------------------------------------------------------
# Creates a list containing names of people who contributed to the project, for use
# in release notes. The names come from the author field on the commits between
# the current revision and the one specified as argument.
#
# Note that the output often requires extra manual processing to remove entries
# that refer to the same person (diacritics vs no diacritics, name vs nickname, etc.).
#
# Usage:
#    <script name>.sh [--yes] <revision> [<target-ref>]
#
# Arguments:
#    --yes, -y     - Skip confirmation prompt
#    <revision>    - Starting revision (e.g., v0.8.32)
#    <target-ref>  - Target ref to compare against (default: develop)
#                    Examples: develop, origin/develop, upstream/develop
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

set -euo pipefail

script_dir=$(dirname "$0")
# shellcheck source=scripts/common.sh
source "${script_dir}/common.sh"

skip_confirm=false
args=()
while [[ $# -gt 0 ]]
do
    case "$1" in
        --yes|-y)
            skip_confirm=true
            ;;
        -*)
            fail "Unknown option: $1. Usage: $0 [--yes] <revision> [<target-ref>]."
            ;;
        *)
            args+=("$1")
            ;;
    esac
    shift
done

(( ${#args[@]} >= 1 && ${#args[@]} <= 2 )) || fail "Wrong number of arguments. Usage: $0 [--yes] <revision> [<target-ref>]."

revision="${args[0]}"
# Default to local develop and its configured tracking branch
target_ref="${args[1]:-develop}"

git rev-parse --verify "$revision" > /dev/null 2>&1 || fail "Invalid revision: $revision"
git rev-parse --verify "$target_ref" > /dev/null 2>&1 || fail "Invalid target ref: $target_ref"

upstream=$(git rev-parse --abbrev-ref "${target_ref}@{upstream}" 2>/dev/null || true)
printWarning "Listing contributors from ${revision} to ${target_ref}${upstream:+ (tracking: $upstream)}..."

if [[ "$skip_confirm" != true ]]; then
    read -r -p "Continue? [y/N] " response
    [[ "$response" =~ ^[Yy]$ ]] || exit 0
fi

# NOTE: Commas are removed from any names containing them. It would look confusing otherwise, given
# that the list is delimited by commas. Hopefully no contributor uses a comma as their nickname.
git shortlog --summary "${revision}..${target_ref}" |
    cut --field 2 |
    tr --delete , |
    sort |
    uniq |
    paste --serial --delimiter=, |
    sed -e 's/,/, /g'
