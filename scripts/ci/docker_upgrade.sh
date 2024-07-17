#!/usr/bin/env bash
set -e

function error
{
  echo >&2 "ERROR: ${1} Aborting." && false
}

function warning
{
  echo >&2 "WARNING: ${1}"
}

[[ $# == 3 ]] || error "Expected exactly 3 parameters: '${0} <IMAGE_NAME> <IMAGE_VARIANT> <DOCKER_REPOSITORY>'."

IMAGE_NAME="${1}"
IMAGE_VARIANT="${2}"
DOCKER_REPOSITORY="${3}"
DOCKERFILE="scripts/docker/${IMAGE_NAME}/Dockerfile.${IMAGE_VARIANT}"

echo "-- check_dockerfile_was_changed"

# exit, if the dockerfile was not changed.
if git diff --quiet origin/develop HEAD -- "${DOCKERFILE}"; then
  echo "${DOCKERFILE} was not changed. Nothing to do."
  exit 0
fi

echo "-- check_version"

PREV_VERSION=$(git diff origin/develop HEAD -- "${DOCKERFILE}" | grep -e '^\s*-LABEL\s\+version=".*"\s*$' | awk -F'"' '{ print $2 }')
NEXT_VERSION=$(git diff origin/develop HEAD -- "${DOCKERFILE}" | grep -e '^\s*+LABEL\s\+version=".*"\s*$' | awk -F'"' '{ print $2 }')

[[ $NEXT_VERSION != "" ]] || error "No version label defined in Dockerfile. You may need to add 'LABEL version' in '${DOCKERFILE}'."

[[ $PREV_VERSION != "" ]] || {
  warning "no previous version found. Will set \$PREV_VERSION = 0."
  PREV_VERSION=0
}

if [[ $((PREV_VERSION + 1)) != $((NEXT_VERSION)) ]]; then
  error "Version label in Dockerfile was not incremented. You may need to change 'LABEL version' in '${DOCKERFILE}'."
fi

echo "-- build_docker"

# This is a workaround: we run `docker build` twice to prevent the `layer does not exist` problem.
# See https://github.com/moby/moby/issues/37965.
docker build "scripts/docker/${IMAGE_NAME}" --file "scripts/docker/${IMAGE_NAME}/Dockerfile.${IMAGE_VARIANT}" --tag "${IMAGE_NAME}" ||
  docker build "scripts/docker/${IMAGE_NAME}" --file "scripts/docker/${IMAGE_NAME}/Dockerfile.${IMAGE_VARIANT}" --tag "${IMAGE_NAME}"

echo "-- test_docker @ '${PWD}'"

# NOTE: Since /root/project/ is a dir from outside the container and the owner of the files is different,
# git show in the script refuses to work. It must be marked as safe to use first.
# See https://github.blog/2022-04-12-git-security-vulnerability-announced/
docker run \
  --rm \
  --volume "${PWD}:/root/project" \
  "${IMAGE_NAME}" \
  bash -c "
    git config --global --add safe.directory /root/project &&
    /root/project/scripts/ci/${IMAGE_NAME}_test_${IMAGE_VARIANT}.sh
  "

echo "-- push_docker"

VERSION=$(docker inspect --format='{{.Config.Labels.version}}' "${IMAGE_NAME}")
DOCKER_IMAGE_ID="${DOCKER_REPOSITORY}:${IMAGE_VARIANT}"

docker tag "${IMAGE_NAME}" "${DOCKER_IMAGE_ID}-${VERSION}"
docker push "${DOCKER_IMAGE_ID}-${VERSION}"

REPO_DIGEST=$(docker inspect --format='{{.RepoDigests}}' "${DOCKER_IMAGE_ID}-${VERSION}")

echo "DOCKER_IMAGE=${DOCKER_IMAGE_ID}-${VERSION}" >> "$GITHUB_ENV"
echo "DOCKER_REPO_DIGEST=${REPO_DIGEST}" >> "$GITHUB_ENV"
