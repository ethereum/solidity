#!/usr/bin/env bash
set -eo pipefail

# This is a regression test against https://github.com/ethereum/solidity/issues/14640
# The bug caused the block deduplicator to incorrectly merge two blocks which have
# verbatim items surrounded by identical opcodes. Due to the bug, the contents of
# the verbatim were ignored and the blocks merged into a single one.

# shellcheck source=scripts/common.sh
source "${REPO_ROOT}/scripts/common.sh"
# shellcheck source=scripts/common_cmdline.sh
source "${REPO_ROOT}/scripts/common_cmdline.sh"

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
YUL_SOURCE="${SCRIPT_DIR}/verbatim_inside_identical_blocks.yul"

VERBATIM_OCCURRENCES=$(< "$YUL_SOURCE" "$SOLC" --strict-assembly - --optimize --asm | grep -e "verbatim" -c)

[[ $VERBATIM_OCCURRENCES == 2 ]] || assertFail "Incorrect number of verbatim items in assembly."
