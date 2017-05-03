#!/usr/bin/env sh

set -e

docker build -t ethereum/solc:build -f scripts/Dockerfile .
tmp_container=$(docker create ethereum/solc:build sh)
mkdir -p upload
docker cp ${tmp_container}:/usr/bin/solc upload/solc-static-linux
