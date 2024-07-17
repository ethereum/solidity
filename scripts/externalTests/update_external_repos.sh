#!/usr/bin/env bash

#------------------------------------------------------------------------------
# Clones external test repositories from solidity-external-tests organization
# and for each of them pulls latest upstream changes from the main branch and
# pushes them to our fork.
#
# The script assumes that the current user has write access to
# solidity-external-tests and that git is configured to be able to push there
# without specifying the password (e.g. with the key already unlocked and loaded
# into ssh-agent). Otherwise git will keep asking for password for each repository.
#
# Usage:
#
#    ./update_external_repos.sh [<target_dir>]
#
# <target_dir>: directory where the clones of the repositories are stored.
#               If omitted, a temporary directory will be created.
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
# (c) 2022 solidity contributors.
#------------------------------------------------------------------------------

set -euo pipefail

target_dir="${1:-$(mktemp -d -t update_external_repos_XXXXXX)}"

function clone_repo
{
    local upstream_user="$1"
    local upstream_repo="$2"
    local fork_name="${3:-$upstream_repo}"

    if [[ ! -d $fork_name ]]; then
        git clone "git@github.com:solidity-external-tests/${fork_name}.git" --no-checkout
    else
        echo "Reusing existing repo: ${fork_name}."
    fi

    pushd "$fork_name" > /dev/null
    git remote rm upstream 2> /dev/null || true
    git remote add upstream "https://github.com/${upstream_user}/${upstream_repo}"
    popd > /dev/null
}

function sync_branch
{
    local fork_name="$1"
    local branch="$2"

    echo "${fork_name}: syncing branch ${branch}..."
    pushd "$fork_name" > /dev/null
    git fetch upstream "$branch" --quiet
    git checkout -B "$branch" --track "upstream/$branch" --quiet
    git merge "upstream/${branch}" --ff-only --quiet
    git push origin "$branch"
    popd > /dev/null
}

mkdir -p "$target_dir"

echo "Entering ${target_dir}"
cd "$target_dir"

clone_repo brinktrade        brink-core
clone_repo dapphub           dappsys-monolithic
clone_repo element-fi        elf-contracts
clone_repo ensdomains        ens-contracts
clone_repo euler-xyz         euler-contracts
clone_repo cowprotocol       contracts                 gp2-contracts
clone_repo gnosis            mock-contract
clone_repo gnosis            util-contracts
clone_repo JoinColony        colonyNetwork
clone_repo mycelium-ethereum perpetual-pools-contracts
clone_repo OpenZeppelin      openzeppelin-contracts
clone_repo paulrberg         prb-math
clone_repo pooltogether      v4-core                   pooltogether-v4-core
clone_repo safe-global       safe-contracts
clone_repo smartcontractkit  chainlink
clone_repo sushiswap         trident
clone_repo Uniswap           v2-core                   uniswap-v2-core
clone_repo Uniswap           v3-core                   uniswap-v3-core
clone_repo wighawag          bleeps
clone_repo yieldprotocol     yield-liquidator-v2

sync_branch brink-core                master
sync_branch dappsys-monolithic        master
sync_branch elf-contracts             main
sync_branch ens-contracts             master
sync_branch euler-contracts           master
sync_branch gp2-contracts             main
sync_branch mock-contract             master
sync_branch util-contracts            main
sync_branch colonyNetwork             develop
sync_branch perpetual-pools-contracts develop
sync_branch openzeppelin-contracts    master
sync_branch prb-math                  main
sync_branch pooltogether-v4-core      master
sync_branch safe-contracts            main
sync_branch chainlink                 develop
sync_branch trident                   master
sync_branch uniswap-v2-core           master
sync_branch uniswap-v3-core           main
sync_branch bleeps                    main
sync_branch yield-liquidator-v2       master
