#!/usr/bin/env bash

set -e

REPO_ROOT="$(dirname "$0")"/../..
REPO_ROOT=$(realpath "${REPO_ROOT}")
IGNORE_FILENAME="ignore.txt"
IGNORE_FILE="${REPO_ROOT}/scripts/chk_shellscripts/${IGNORE_FILENAME}"

FOUND_FILES_TMP=$(mktemp)
IGNORE_FILES_TMP=$(mktemp)
trap 'rm -f ${FOUND_FILES_TMP} ; rm -f ${IGNORE_FILES_TMP}' EXIT

sort < "${IGNORE_FILE}" >"${IGNORE_FILES_TMP}"
cd "${REPO_ROOT}"
find . -type f -name "*.sh" | sort >"${FOUND_FILES_TMP}"

SHELLCHECK=${SHELLCHECK:-"$(command -v -- shellcheck)"}
if [ ! -f "${SHELLCHECK}" ]; then
  echo "error: shellcheck '${SHELLCHECK}' not found."
  exit 1
fi

FILES=$(join -v2 "${IGNORE_FILES_TMP}" "${FOUND_FILES_TMP}")

# shellcheck disable=SC2086
"${SHELLCHECK}" ${FILES[*]}
