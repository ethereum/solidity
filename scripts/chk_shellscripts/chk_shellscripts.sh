#!/usr/bin/env bash

set -eu

REPO_ROOT="$(dirname "$0")"/../..
REPO_ROOT=$(realpath "${REPO_ROOT}")

cd "${REPO_ROOT}"

SHELLCHECK=${SHELLCHECK:-"$(command -v -- shellcheck)"}
if [ ! -f "${SHELLCHECK}" ]; then
    echo "error: shellcheck '${SHELLCHECK}' not found."
    exit 1
fi

mapfile -t FILES < <(find . -type f -name "*.sh")
"${SHELLCHECK}" "${FILES[@]}"
