#!/usr/bin/env bash
set -euo pipefail

# shellcheck source=scripts/common.sh
source "${REPO_ROOT}/scripts/common.sh"

printTask "Testing AST import/export..."
SOLTMPDIR=$(mktemp -d)
(
    cd "$SOLTMPDIR"
    if ! "$REPO_ROOT/scripts/ASTImportTest.sh" ast
    then
        rm -r "$SOLTMPDIR"
        fail
    fi
)
rm -r "$SOLTMPDIR"
