#!/usr/bin/env bash
set -euo pipefail

# shellcheck source=scripts/common.sh
source "${REPO_ROOT}/scripts/common.sh"

printTask "Testing AST export with stop-after=parsing..."
"$REPO_ROOT/test/stopAfterParseTests.sh"
