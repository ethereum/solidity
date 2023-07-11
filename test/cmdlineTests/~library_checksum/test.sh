#!/usr/bin/env bash
set -euo pipefail

# shellcheck source=scripts/common.sh
source "${REPO_ROOT}/scripts/common.sh"

echo '' | msg_on_error --no-stdout "$SOLC" - --link --libraries a=0x90f20564390eAe531E810af625A22f51385Cd222
echo '' | "$SOLC" - --link --libraries a=0x80f20564390eAe531E810af625A22f51385Cd222 &>/dev/null && \
    fail "solc --link did not reject a library address with an invalid checksum."

exit 0
