#!/usr/bin/env sh

set -e

if [ -z "$1" ]
then
    echo "Usage: $0 <tag/branch>"
    exit 1
fi
image="ethereum/solc"
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

tag_and_push()
{
    docker tag "$image:$1" "$image:$2"
    docker push "$image:$2"
}

rm -rf .git
docker build -t "$image":build -f scripts/Dockerfile .
tmp_container=$(docker create "$image":build sh)

# Alpine image
mkdir -p upload
docker cp ${tmp_container}:/usr/bin/solc upload/solc-static-linux
docker build -t "$image":build-alpine -f scripts/Dockerfile_alpine .

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
