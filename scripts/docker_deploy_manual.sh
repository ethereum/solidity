#!/usr/bin/env sh

set -e

if [ -z "$1" ]
then
    echo "Usage: $0 <tag/branch>"
    exit 1
fi
branch="$1"

#docker login

DIR=$(mktemp -d)
(
cd "$DIR"

git clone --depth 2 https://github.com/ethereum/solidity.git -b "$branch"
cd solidity
commithash=$(git rev-parse --short=8 HEAD)
echo -n "$commithash" > commit_hash.txt
version=$($(dirname "$0")/get_version.sh)
if [ "$branch" = "release" -o "$branch" = v"$version" ]
then
    echo -n > prerelease.txt
else
    date -u +"nightly.%Y.%-m.%-d" > prerelease.txt
fi

rm -rf .git
docker build -t ethereum/solc:build -f scripts/Dockerfile .
tmp_container=$(docker create ethereum/solc:build sh)
if [ "$branch" = "develop" ]
then
    docker tag ethereum/solc:build ethereum/solc:nightly;
    docker tag ethereum/solc:build ethereum/solc:nightly-"$version"-"$commithash"
    docker push ethereum/solc:nightly-"$version"-"$commithash";
    docker push ethereum/solc:nightly;
elif [ "$branch" = v"$version" ]
then
    docker tag ethereum/solc:build ethereum/solc:stable;
    docker tag ethereum/solc:build ethereum/solc:"$version";
    docker push ethereum/solc:stable;
    docker push ethereum/solc:"$version";
else
    echo "Not publishing docker image from branch or tag $branch"
fi
)
rm -rf "$DIR"
