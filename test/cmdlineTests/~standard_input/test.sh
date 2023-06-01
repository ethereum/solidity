#!/usr/bin/env bash
set -euo pipefail

# shellcheck source=scripts/common.sh
source "${REPO_ROOT}/scripts/common.sh"

set +e
output=$("$SOLC" --bin 2>&1)
result=$?
set -e

# This should fail
if [[ ! ("$output" =~ "No input files given") || ($result == 0) ]]
then
    fail "Incorrect response to empty input arg list: $output"
fi

# The contract should be compiled
if ! echo 'contract C {}' | msg_on_error --no-stderr "$SOLC" - --bin | grep -q "<stdin>:C"
then
    fail "Failed to compile a simple contract from standard input"
fi

# This should not fail
echo '' | msg_on_error --silent --msg "Incorrect response to --ast-compact-json option with empty stdin" \
    "$SOLC" --ast-compact-json -
