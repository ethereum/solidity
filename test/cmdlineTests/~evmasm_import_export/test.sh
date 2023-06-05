#!/usr/bin/env bash
set -euo pipefail

# shellcheck source=scripts/common.sh
source "${REPO_ROOT}/scripts/common.sh"

SOLTMPDIR=$(mktemp -d -t "cmdline-test-evmasm-import-export-XXXXXX")
(
    cd "$SOLTMPDIR"
    if ! "$REPO_ROOT/scripts/ASTImportTest.sh" evm-assembly
    then
        rm -r "$SOLTMPDIR"
        fail
    fi
)
rm -r "$SOLTMPDIR"
