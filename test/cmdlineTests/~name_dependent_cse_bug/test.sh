#!/usr/bin/env bash
set -eo pipefail

# This is a regression test against https://github.com/ethereum/solidity/issues/14494
# Due to the bug, a decision about which variable to use to replace a subexpression in CSE would
# depend on sorting order of variable names. A variable not being used as a replacement could lead
# to it being unused in general and removed by Unused Pruner. This would show up as a difference
# in the bytecode.

# shellcheck source=scripts/common.sh
source "${REPO_ROOT}/scripts/common.sh"
# shellcheck source=scripts/common_cmdline.sh
source "${REPO_ROOT}/scripts/common_cmdline.sh"

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)

function assemble_with_variable_name {
    local input_file="$1"
    local variable_name="$2"

    sed -e "s|__placeholder__|${variable_name}|g" "$input_file" | msg_on_error --no-stderr \
        "$SOLC" --strict-assembly - --optimize --debug-info none |
            stripCLIDecorations
}

diff_values \
    "$(assemble_with_variable_name "${SCRIPT_DIR}/cse_bug.yul" _1)" \
    "$(assemble_with_variable_name "${SCRIPT_DIR}/cse_bug.yul" _2)"
