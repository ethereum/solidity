#!/usr/bin/env bash

#------------------------------------------------------------------------------
# Downloads and configures external projects used for benchmarking by external.sh.
#
# By default the download location is the benchmarks/ dir at the repository root.
# A different directory can be provided via the BENCHMARK_DIR variable.
#
# Dependencies: foundry, git.
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
# (c) 2024 solidity contributors.
#------------------------------------------------------------------------------

set -euo pipefail

repo_root=$(cd "$(dirname "$0")/../../" && pwd)
BENCHMARK_DIR="${BENCHMARK_DIR:-${repo_root}/benchmarks}"

function neutralize_version_pragmas {
    find . -name '*.sol' -type f -print0 | xargs -0 \
        sed -i -E -e 's/pragma solidity [^;]+;/pragma solidity *;/'
}

function neutralize_via_ir {
    sed -i '/^via_ir\s*=.*$/d' foundry.toml
}

mkdir -p "$BENCHMARK_DIR"
cd "$BENCHMARK_DIR"

if [[ ! -e openzeppelin/ ]]; then
    git clone --depth=1 https://github.com/OpenZeppelin/openzeppelin-contracts openzeppelin/ --branch v5.0.2
    pushd openzeppelin/
    forge install
    neutralize_via_ir
    popd
else
    echo "Skipped openzeppelin/. Already exists."
fi

if [[ ! -e uniswap-v4/ ]]; then
    git clone --single-branch https://github.com/Uniswap/v4-core uniswap-v4/
    pushd uniswap-v4/
    git checkout ae86975b058d386c9be24e8994236f662affacdb # branch main as of 2024-06-06
    forge install
    neutralize_via_ir
    popd
else
    echo "Skipped uniswap-v4/. Already exists."
fi

if [[ ! -e seaport/ ]]; then
    git clone --single-branch https://github.com/ProjectOpenSea/seaport
    pushd seaport/
    # NOTE: Can't select the tag with `git clone` because a branch of the same name exists.
    git checkout tags/1.6
    forge install
    neutralize_via_ir
    neutralize_version_pragmas
    popd
else
    echo "Skipped seaport/. Already exists."
fi

if [[ ! -e eigenlayer/ ]]; then
    git clone --depth=1 https://github.com/Layr-Labs/eigenlayer-contracts eigenlayer/ --branch v0.3.0-holesky-rewards
    pushd eigenlayer/
    neutralize_via_ir
    forge install
    popd
else
    echo "Skipped eigenlayer/. Already exists."
fi

if [[ ! -e sablier-v2/ ]]; then
    git clone --depth=1 https://github.com/sablier-labs/v2-core sablier-v2/ --branch v1.1.2
    pushd sablier-v2/
    # NOTE: To avoid hard-coding dependency versions here we'd have to install them from npm
    forge install --no-commit \
        foundry-rs/forge-std@v1.5.6 \
        OpenZeppelin/openzeppelin-contracts@v4.9.2 \
        PaulRBerg/prb-math@v4.0.2 \
        PaulRBerg/prb-test@v0.6.4 \
        evmcheb/solarray@a547630 \
        Vectorized/solady@v0.0.129
   cat <<EOF > remappings.txt
@openzeppelin/contracts/=lib/openzeppelin-contracts/contracts/
forge-std/=lib/forge-std/
@prb/math/=lib/prb-math/
@prb/test/=lib/prb-test/
solarray/=lib/solarray/
solady/=lib/solady/
EOF
    neutralize_via_ir
    popd
else
    echo "Skipped sablier-v2/. Already exists."
fi
