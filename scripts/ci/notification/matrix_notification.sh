#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(dirname "$0")"

function fail() {
    printf '%s\n' "$1" >&2
    exit 1
}

function notify() {
    [[ -z "$1" ]] && fail "Event type not provided."
    [[ "$1" != "failure" && "$1" != "success" && "$1" != "release" ]] && fail "Wrong event type."
    local event="$1"
    local formatted_message

    # FIXME: Checking $CIRCLE_PULL_REQUEST would be better than hard-coding branch names
    # but it's broken. CircleCI associates runs on develop/breaking with random old PRs.
    [[ "$BRANCH" == "develop" || "$BRANCH" == "breaking" ]] || { echo "Running on a PR or a feature branch - notification skipped."; exit 0; }

    # The release notification only makes sense on tagged commits. If the commit is untagged, just bail out.
    [[ "$event" == "release" ]] && { [[ $TAG != "" ]] || { echo "Not a tagged commit - notification skipped."; exit 0; } }

    formatted_message="$(format_predefined_message "$event")"

    curl "https://${MATRIX_SERVER}/_matrix/client/v3/rooms/${MATRIX_NOTIFY_ROOM_ID}/send/m.room.message" \
        --request POST \
        --include \
        --fail \
        --header "Content-Type: application/json" \
        --header "Accept: application/json" \
        --header "Authorization: Bearer ${MATRIX_ACCESS_TOKEN}" \
        --data "$formatted_message"
}

function circleci_workflow_name() {
    # Workflow name is not exposed as an env variable. Has to be queried from the API.
    # The name is not critical so if anything fails, use the raw workflow ID as a fallback.
    local workflow_info
    workflow_info=$(curl --silent "https://circleci.com/api/v2/workflow/${CIRCLE_WORKFLOW_ID}") || true
    echo "$workflow_info" | grep -E '"\s*name"\s*:\s*".*"' | cut -d \" -f 4 || echo "$CIRCLE_WORKFLOW_ID"
}

function circleci_job_name() {
    (( CIRCLE_NODE_TOTAL == 1 )) && echo "${CIRCLE_JOB}" && return
    (( CIRCLE_NODE_TOTAL != 1 )) && echo "${CIRCLE_JOB} (run $((CIRCLE_NODE_INDEX + 1))/${CIRCLE_NODE_TOTAL})"
}

# Currently matrix only supports html formatted body,
# see: https://spec.matrix.org/v1.6/client-server-api/#mtext
#
# Eventually, the matrix api will have support for better format options and `formatted_body` may not be necessary anymore:
# https://github.com/matrix-org/matrix-spec-proposals/pull/1767
function format_predefined_message() {
    [[ -z "$1" ]] && fail "Event type not provided."
    local event="$1"
    local template

    [[ "$event" == "failure" ]] && template="${SCRIPT_DIR}/templates/build_fail.json"
    [[ "$event" == "success" ]] && template="${SCRIPT_DIR}/templates/build_success.json"
    [[ "$event" == "release" ]] && template="${SCRIPT_DIR}/templates/build_release.json"

    [[ -z "$template" ]] && fail "Message template for event [$event] not defined."

    # shellcheck disable=SC2016
    sed -e 's|${WORKFLOW_NAME}|'"${WORKFLOW_NAME}"'|' \
        -e 's|${JOB}|'"${JOB}"'|' \
        -e 's|${BRANCH}|'"${BRANCH}"'|' \
        -e 's|${TAG}|'"${TAG}"'|' \
        -e 's|${BUILD_URL}|'"${BUILD_URL}"'|' \
        -e 's|${BUILD_NUM}|'"${BUILD_NUM}"'|' \
        "$template"
}

# Set message environment variables based on CI backend
if [[ "$CIRCLECI" = true ]] ; then
    BRANCH="$CIRCLE_BRANCH"
    TAG="${CIRCLE_TAG:-}"
    BUILD_URL="$CIRCLE_BUILD_URL"
    BUILD_NUM="$CIRCLE_BUILD_NUM"
    WORKFLOW_NAME="$(circleci_workflow_name)"
    JOB="$(circleci_job_name)"
fi

notify "$@"
