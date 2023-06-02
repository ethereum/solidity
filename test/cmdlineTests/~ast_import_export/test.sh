#!/usr/bin/env bash
set -euo pipefail

# shellcheck source=scripts/common.sh
source "${REPO_ROOT}/scripts/common.sh"

SOLTMPDIR=$(mktemp -d -t "cmdline-test-ast-import-export-XXXXXX")
cd "$SOLTMPDIR"
if ! "$REPO_ROOT/scripts/ASTImportTest.sh" ast
then
    rm -r "$SOLTMPDIR"
    fail
fi
rm -r "$SOLTMPDIR"
