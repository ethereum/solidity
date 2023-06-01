#!/usr/bin/env bash
set -euo pipefail

# shellcheck source=scripts/common.sh
source "${REPO_ROOT}/scripts/common.sh"

printTask "Testing unknown options..."
set +e
output=$("$SOLC" --allow=test 2>&1)
failed=$?
set -e

if [ "$output" == "unrecognised option '--allow=test'" ] && [ $failed -ne 0 ]
then
    echo "Passed"
else
    fail "Incorrect response to unknown options: $output"
fi
