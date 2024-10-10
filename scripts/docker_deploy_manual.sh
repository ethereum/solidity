#!/usr/bin/env bash

set -e

REPO_ROOT="$(dirname "$0")/.."
# shellcheck source=scripts/common.sh
source "${REPO_ROOT}/scripts/common.sh"

image="ethereum/solc"

if (( $# < 1 || $# > 3 )); then
    fail "Usage: $0 <tag/branch> [repo URL] [--no-push]"
fi

branch="$1"
repo_url="${2:-https://github.com/ethereum/solidity.git}"

if (( $# >= 3 )); then
    [[ $3 == --no-push ]] || fail "Invalid flag: $3. Expected --no-push."
    publish=false
else
    publish=true
fi

#docker login

DIR=$(mktemp -d)
(
cd "$DIR"

git clone --recursive --depth 2 "$repo_url" -b "$branch" solidity
cd solidity
commithash=$(git rev-parse --short=8 HEAD)
echo -n "$commithash" > commit_hash.txt
version=$("$(dirname "$0")/get_version.sh")
if [ "$branch" = v"$version" ]
then
    echo -n > prerelease.txt
else
    date -u +"nightly.%Y.%-m.%-d" > prerelease.txt
fi

function tag_and_push
{
    docker tag "$image:$1" "$image:$2"
    [[ $publish == false ]] || docker push "$image:$2"
}

rm -rf .git
docker build -t "$image":build -f scripts/Dockerfile . --progress plain
tmp_container=$(docker create "$image":build sh)

# Alpine image
mkdir -p upload
docker cp "${tmp_container}":/usr/bin/solc upload/solc-static-linux
docker build -t "$image":build-alpine -f scripts/Dockerfile_alpine . --progress plain

if [ "$branch" = "develop" ]
then
    tag_and_push build nightly
    tag_and_push build nightly-"$version"-"$commithash"
    tag_and_push build-alpine nightly-alpine
    tag_and_push build-alpine nightly-alpine-"$version"-"$commithash"
elif [ "$branch" = v"$version" ]
then
    tag_and_push build stable
    tag_and_push build "$version"
    tag_and_push build-alpine stable-alpine
    tag_and_push build-alpine "$version"-alpine
else
    echo "Not publishing docker image from branch or tag $branch"
fi
)
rm -rf "$DIR"
