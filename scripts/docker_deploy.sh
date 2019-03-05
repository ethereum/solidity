#!/usr/bin/env sh

set -e

image="ethereum/solc"

tag_and_push()
{
    docker tag "$image:$1" "$image:$2"
    docker push "$image:$2"
}

echo "$DOCKER_PASSWORD" | docker login -u "$DOCKER_USERNAME" --password-stdin
version=$($(dirname "$0")/get_version.sh)
if [ "$TRAVIS_BRANCH" = "develop" ]
then
    tag_and_push build nightly
    tag_and_push build nightly-"$version"-"$TRAVIS_COMMIT"
    tag_and_push build-alpine nightly-alpine
    tag_and_push build-alpine nightly-alpine-"$version"-"$TRAVIS_COMMIT"
elif [ "$TRAVIS_TAG" = v"$version" ]
then
    tag_and_push build stable
    tag_and_push build "$version"
    tag_and_push build-alpine stable-alpine
    tag_and_push build-alpine "$version"-alpine
else
    echo "Not publishing docker image from branch $TRAVIS_BRANCH or tag $TRAVIS_TAG"
fi
