#!/usr/bin/env bash

set -euo pipefail

function help() {
    echo "Downloads binaries created by a CircleCI job to the current working,"
    echo "directory and renames them according to the naming convention used"
    echo "on the Solidity release page on Github."
    echo
    echo "WARNING: The binaries will be overwritten if they already exist."
    echo
    echo
    echo "Usage:"
    echo "    ./$(basename "$0") --help"
    echo "    ./$(basename "$0") workflow_id"
    echo
    echo "    workflow_id      CircleCI workflow ID. Identifies the workflow that contains"
    echo "                     jobs whose artifacts should be downloaded."
    echo
    echo
    echo "Examples:"
    echo "    ./$(basename "$0") --help"
    echo "    ./$(basename "$0") 12345678-90ab-cdef-1234-567890abcdef"
}


# GENERAL UTILITIES

query_api() {
    local api_endpoint="$1"

    curl --fail --silent --show-error "$api_endpoint"
}

fail() {
    local format="$1"

    # shellcheck disable=SC2059
    >&2 printf "ERROR: $format\n" "${@:2}"
    exit 1
}

expect_single_line() {
    local text="$1"

    local line_count; line_count="$(echo "$text" | grep --count "")"
    [[ $text != "" ]] || fail "Expected one line, got zero."
    (( "$line_count" < 2 )) || fail "Expected one line, got %d:\n%s" "$line_count" "$text"
}


# JSON FILTERING

filter_artifacts_by_name()  {
    local artifact_name="$1"

    jq '[ .items[] | select (.path == "'"$artifact_name"'") ]'
}

filter_jobs_by_name()  {
    local job_name="$1"

    jq '[ .items[] | select (.name == "'"$job_name"'") ]'
}


# ENUMERATIONS

target_to_job_name() {
    local target="$1"

    case "$target" in
        linux-amd64)   echo b_ubu_static ;;
        macosx-amd64)  echo b_osx ;;
        windows-amd64) echo b_win_release ;;
        emscripten)    echo b_ems ;;
        *) fail "Invalid target: %s" "$target" ;;
    esac
}

target_to_circleci_artifact_name() {
    local target="$1"

    case "$target" in
        linux-amd64)   echo solc ;;
        macosx-amd64)  echo solc ;;
        windows-amd64) echo upload/bin/solc.exe ;;
        emscripten)    echo soljson.js ;;
        *) fail "Invalid target: %s" "$target" ;;
    esac
}

is_executable() {
    local target="$1"

    case "$target" in
        linux-amd64)   true ;;
        macosx-amd64)  true ;;
        windows-amd64) false ;;
        emscripten)    false ;;
        *) fail "Invalid target: %s" "$target" ;;
    esac
}

target_to_binary_name() {
    local target="$1"

    case "$target" in
        linux-amd64)   echo solc-static-linux ;;
        macosx-amd64)  echo solc-macos ;;
        windows-amd64) echo solc-windows.exe ;;
        emscripten)    echo soljson.js ;;
        *) fail "Invalid target: %s" "$target" ;;
    esac
}


# MAIN LOGIC

query_circleci_artifact_url() {
    local target="$1"
    local build_num="$2"

    local artifact_name; artifact_name="$(target_to_circleci_artifact_name "$target")"
    local artifact_info; artifact_info="$(
        query_api "https://circleci.com/api/v2/project/gh/ethereum/solidity/${build_num}/artifacts" |
        filter_artifacts_by_name "$artifact_name"
    )"

    [[ $artifact_info != "[]" ]] || fail "Artifact '%s' is missing." "$artifact_name"

    local artifact_url; artifact_url="$(echo "$artifact_info" | jq --raw-output '.[].url')"
    expect_single_line "$artifact_url"

    echo "$artifact_url"
}

download_binary() {
    local target="$1"
    local download_url="$2"

    local target_path; target_path="$(target_to_binary_name "$target")"

    # If the target exists we ovewrite it. As a special case, if it's a symlink, remove it
    # so that we only change link not the file it links to.
    [[ ! -L "$target_path" ]] || rm "$target_path"

    echo "Downloading release binary from ${download_url} into ${target_path}"
    curl "$download_url" --output "$target_path" --location --no-progress-meter --create-dirs

    ! is_executable "$target" || chmod +x "$target_path"
}

download_binary_from_circleci() {
    local target="$1"
    local workflow_job_info="$2"

    local job_name; job_name="$(target_to_job_name "$target")"
    local job_info; job_info="$(
        echo "$workflow_job_info" |
        filter_jobs_by_name "$job_name"
    )"
    [[ $job_info != "[]" ]] || fail "Job '%s' not found." "$job_name"

    local build_num; build_num="$(echo "$job_info" | jq --raw-output '.[].job_number')"
    expect_single_line "$build_num"

    local artifact_url; artifact_url="$(query_circleci_artifact_url "$target" "$build_num")"
    download_binary "$target" "$artifact_url"
}

download_binaries() {
    local workflow_id="$1"

    echo "===> DOWNLOADING BINARIES FOR WORKFLOW ${workflow_id} FROM CIRCLECI"

    local workflow_job_info; workflow_job_info="$(
        query_api "https://circleci.com/api/v2/workflow/${workflow_id}/job"
    )"

    local release_targets=(
        linux-amd64
        windows-amd64
        macosx-amd64
        emscripten
    )

    for target in "${release_targets[@]}"; do
        download_binary_from_circleci "$target" "$workflow_job_info"
    done
}

main() {
    if (( $# > 0 )) && [[ $1 == --help ]]; then
        help
        exit 0
    fi

    (( $# >= 1 )) || fail "Too few arguments"
    (( $# <= 1 )) || fail "Too many arguments"

    local workflow_id="${1:-$PWD}"

    download_binaries "$workflow_id"
}

main "$@"
