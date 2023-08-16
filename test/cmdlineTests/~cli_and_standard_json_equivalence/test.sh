#!/usr/bin/env bash
set -euo pipefail

# shellcheck source=scripts/common.sh
source "${REPO_ROOT}/scripts/common.sh"
# shellcheck source=scripts/common_cmdline.sh
source "${REPO_ROOT}/scripts/common_cmdline.sh"

function test_cli_and_standard_json_equivalence
{
    (( $# == 5 )) || assertFail
    local cli_options="$1"
    local selected_cli_output="$2"
    local standard_json_settings="$3"
    local selected_standard_json_output="$4"
    local input_file_relative_path="$5"

    # CLI normalizes paths, Standard JSON uses them as is. Using paths that would change under this
    # normalization will make the comparison fail. To avoid this use already normalized paths.
    # The sanity check below should reject most of these by disallowing absolute paths, relative
    # paths with ./ or ../ segments and paths with redundant slashes, but keep in mind it's not foolproof.
    [[ $input_file_relative_path =~ ^/|^\.$|\./|^\.\.$|\.\./|// ]] && assertfail

    local cli_output standard_json_output
    cli_output=$(
        # shellcheck disable=SC2086 # Intentionally unquoted. May contain multiple options.
        msg_on_error --no-stderr \
            "$SOLC" $cli_options "$selected_cli_output" "$input_file_relative_path"
    )
    standard_json_output=$(
        singleContractOutputViaStandardJSON \
            Solidity \
            "$selected_standard_json_output" \
            "$standard_json_settings" \
            "$input_file_relative_path"
    )

    diff_values \
        "$(echo "$cli_output" | stripCLIDecorations | stripEmptyLines)" \
        "$(echo "$standard_json_output" | stripEmptyLines)" \
        --ignore-space-change \
        --ignore-blank-lines
}

cd "$REPO_ROOT"

printTask "    - --optimize vs optimizer.enabled: true (--asm output)"
test_cli_and_standard_json_equivalence \
    '--optimize' \
    '--asm' \
    '"optimizer": {"enabled": true}' \
    'evm.assembly' \
    "test/libsolidity/semanticTests/various/erc20.sol"

printTask "    - --optimize-yul vs optimizer.details.yul: true (--asm output)"
test_cli_and_standard_json_equivalence \
    '--optimize-yul' \
    '--asm' \
    '"optimizer": {"enabled": false, "details": {"yul": true}}' \
    'evm.assembly' \
    "test/libsolidity/semanticTests/various/erc20.sol"
