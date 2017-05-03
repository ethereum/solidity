#!/usr/bin/env sh

set -e

docker login -u="$DOCKER_USERNAME" -p="$DOCKER_PASSWORD";
version=$(grep -oP "PROJECT_VERSION \"?\K[0-9.]+(?=\")"? $(dirname "$0")/../CMakeLists.txt)
if [ "$TRAVIS_BRANCH" = "develop" ]
then
    docker tag ethereum/solc:build ethereum/solc:nightly;
    docker tag ethereum/solc:build ethereum/solc:nightly-"$version"-"$TRAVIS_COMMIT"
    docker push ethereum/solc:nightly-"$version"-"$TRAVIS_COMMIT";
    docker push ethereum/solc:nightly;
elif [ "$TRAVIS_TAG" = v"$version" ]
then
    docker tag ethereum/solc:build ethereum/solc:stable;
    docker tag ethereum/solc:build ethereum/solc:"$version";
    docker push ethereum/solc:stable;
    docker push ethereum/solc:"$version";
else
    echo "Not publishing docker image from branch $TRAVIS_BRANCH or tag $TRAVIS_TAG"
fi
