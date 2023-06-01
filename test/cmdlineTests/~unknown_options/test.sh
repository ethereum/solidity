#!/usr/bin/env bash
set -euo pipefail

# shellcheck source=scripts/common.sh
source "${REPO_ROOT}/scripts/common.sh"

set +e
output=$("$SOLC" --allow=test 2>&1)
failed=$?
set -e

if [[ $output != "unrecognised option '--allow=test'" ]] || (( failed == 0 ))
then
    fail "Incorrect response to unknown options: $output"
fi
