#!/usr/bin/env sh

# Script to build the eth binary from latest develop
# for ubuntu trusty and ubuntu artful.
# Requires docker.

set -e

REPO_ROOT="$(dirname "$0")"/../..

for rel in artful trusty
do
    docker build -t eth_$rel -f "$REPO_ROOT"/scripts/cpp-ethereum/eth_$rel.docker .
    tmp_container=$(docker create eth_$rel sh)
    echo "Built eth ($rel) at $REPO_ROOT/build/eth_$rel"
    docker cp ${tmp_container}:/build/eth/eth "$REPO_ROOT"/build/eth_$rel
done