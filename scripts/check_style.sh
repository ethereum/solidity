#!/usr/bin/env bash

set -eu

ERROR_LOG="$(mktemp -t check_style_XXXXXX.log)"

EXCLUDE_FILES=(
    "libsolutil/picosha2.h"
    "test/cmdlineTests/strict_asm_only_cr/input.yul"
    "test/cmdlineTests/strict_asm_only_cr/err"
    "test/libsolutil/UTF8.cpp"
    "test/libsolidity/syntaxTests/license/license_cr_endings.sol"
    "test/libsolidity/syntaxTests/license/license_crlf_endings.sol"
    "test/libsolidity/syntaxTests/license/license_whitespace_trailing.sol"
    "test/scripts/fixtures/smt_contract_with_crlf_newlines.sol"
    "test/scripts/fixtures/smt_contract_with_cr_newlines.sol"
    "test/scripts/fixtures/smt_contract_with_mixed_newlines.sol"
)
EXCLUDE_FILES_JOINED=$(printf "%s\|" "${EXCLUDE_FILES[@]}")
EXCLUDE_FILES_JOINED=${EXCLUDE_FILES_JOINED%??}

(
REPO_ROOT="$(dirname "$0")"/..
cd "$REPO_ROOT" || exit 1

WHITESPACE=$(git grep -n -I -E "^.*[[:space:]]+$" |
    grep -v "test/libsolidity/ASTJSON\|test/libsolidity/ASTRecoveryTests\|test/compilationTests/zeppelin/LICENSE\|${EXCLUDE_FILES_JOINED}" || true
)

if [[ "$WHITESPACE" != "" ]]
then
    echo "Error: Trailing whitespace found:" | tee -a "$ERROR_LOG"
    echo "$WHITESPACE" | tee -a "$ERROR_LOG"
    scripts/ci/post_style_errors_on_github.sh "$ERROR_LOG"
    exit 1
fi

function preparedGrep
{
    git grep -nIE "$1" -- '*.h' '*.cpp' | grep -v "${EXCLUDE_FILES_JOINED}"
    return $?
}

FORMATERROR=$(
(
    preparedGrep "#include \"" | grep -E -v -e "license.h" -e "BuildInfo.h"  # Use include with <> characters
    preparedGrep "\<(if|for|while|switch)\(" # no space after "if", "for", "while" or "switch"
    preparedGrep "\<for\>\s*\([^=]*\>\s:\s.*\)" # no space before range based for-loop
    preparedGrep "\<if\>\s*\(.*\)\s*\{\s*$" # "{\n" on same line as "if"
    preparedGrep "namespace .*\{"
    preparedGrep "[,\(<]\s*const " # const on left side of type
    preparedGrep "^\s*(static)?\s*const " # const on left side of type (beginning of line)
    preparedGrep "^ [^*]|[^*] 	|	 [^*]" # uses spaces for indentation or mixes spaces and tabs
    preparedGrep "[a-zA-Z0-9_]\s*[&][a-zA-Z_]" | grep -E -v "return [&]" # right-aligned reference ampersand (needs to exclude return)
    # right-aligned reference pointer star (needs to exclude return and comments)
    preparedGrep "[a-zA-Z0-9_]\s*[*][a-zA-Z_]" | grep -E -v -e "return [*]" -e "^* [*]" -e "^*//.*"
) | grep -E -v -e "^[a-zA-Z\./]*:[0-9]*:\s*\/(\/|\*)" -e "^test/" || true
)

if [[ "$FORMATERROR" != "" ]]
then
    echo "Coding style error:" | tee -a "$ERROR_LOG"
    echo "$FORMATERROR" | tee -a "$ERROR_LOG"
    scripts/ci/post_style_errors_on_github.sh "$ERROR_LOG"
    exit 1
fi
)
