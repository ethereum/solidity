#!/usr/bin/env bash
set -euo pipefail

# shellcheck source=scripts/common.sh
source "${REPO_ROOT}/scripts/common.sh"

SOLTMPDIR=$(mktemp -d -t "cmdline-test-update-bugs-by-version-XXXXXX")

cp "${REPO_ROOT}/docs/bugs_by_version.json" "${SOLTMPDIR}/original_bugs_by_version.json"
"${REPO_ROOT}/scripts/update_bugs_by_version.py"

diff --unified "${SOLTMPDIR}/original_bugs_by_version.json" "${REPO_ROOT}/docs/bugs_by_version.json" || \
    fail "The bug list in bugs_by_version.json was out of date and has been updated. Please investigate and submit a bugfix if necessary."

rm -r "$SOLTMPDIR"
