#!/usr/bin/env bash

set -eu

ERROR_LOG="$(mktemp -t check_style_XXXXXX.log)"

if [ "$(uname)" == "Darwin" ]; then
    if ! command -v ggrep &> /dev/null
    then
        brew install grep # install GNU grep on macOS
    fi
    grepCommand="ggrep"
else
    grepCommand="grep"
fi

EXCLUDE_FILES=(
    # The line below is left unquoted to allow the shell globbing path expansion
    test/cmdlineTests/*/{err,output}
    "libsolutil/picosha2.h"
    "test/cmdlineTests/strict_asm_only_cr/input.yul"
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

NAMESPACE_STD_FREE_FILES=(
    libevmasm/*
    liblangutil/*
    libsmtutil/*
    libsolc/*
    libsolidity/analysis/*
    libsolidity/ast/*
    libsolidity/codegen/ir/*
    libsolidity/codegen/*
    libsolidity/experimental/*
    libsolidity/formal/*
    libsolidity/interface/*
    libsolidity/lsp/*
    libsolidity/parsing/*
    libsolutil/*
    libyul/*
    libyul/backends/evm/*
    libyul/optimiser/*
    solc/*
    test/contracts/*
    test/libevmasm/*
    test/liblangutil/*
    test/libsolutil/*
    test/libsolidity/*
    test/libsolidity/analysis/*
    test/libsolidity/interface/*
    test/libsolidity/util/*
    test/libyul/*
    test/solc/*
    test/tools/yulInterpreter/*
)

(
REPO_ROOT="$(dirname "$0")"/..
cd "$REPO_ROOT" || exit 1

WHITESPACE=$(git grep -n -I -E "^.*[[:space:]]+$" |
    ${grepCommand} -v "test/libsolidity/ASTJSON\|test/libsolidity/ASTRecoveryTests\|test/compilationTests/zeppelin/LICENSE\|${EXCLUDE_FILES_JOINED}" || true
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
    git grep -nIE "$1" -- '*.h' '*.cpp' | ${grepCommand} -v "${EXCLUDE_FILES_JOINED}"
    return $?
}

FORMATERROR=$(
(
    preparedGrep "#include \"" | ${grepCommand} -E -v -e "license.h" -e "BuildInfo.h"  # Use include with <> characters
    preparedGrep "\<(if|for|while|switch)\(" # no space after "if", "for", "while" or "switch"
    preparedGrep "\<for\>[[:space:]]*\([^=]*\>[[:space:]]:[[:space:]].*\)" # no space before range based for-loop
    preparedGrep "\<if\>[[:space:]]*\(.*\)[[:space:]]*\{[[:space:]]*$" # "{\n" on same line as "if"
    preparedGrep "namespace .*\{"
    preparedGrep "[,\(<][[:space:]]*const " # const on left side of type
    preparedGrep "^[[:space:]]*(static)?[[:space:]]*const " # const on left side of type (beginning of line)
    preparedGrep "^ [^*]|[^*] 	|	 [^*]" # uses spaces for indentation or mixes spaces and tabs
    preparedGrep "[a-zA-Z0-9_][[:space:]]*[&][a-zA-Z_]" | ${grepCommand} -E -v "return [&]" # right-aligned reference ampersand (needs to exclude return)
    # right-aligned reference pointer star (needs to exclude return and comments)
    preparedGrep "[a-zA-Z0-9_][[:space:]]*[*][a-zA-Z_]" | ${grepCommand} -E -v -e "return [*]" -e ":[[:space:]]*[*]" -e ".*//.*"
    # unqualified move()/forward() checks, i.e. make sure that std::move() and std::forward() are used instead of move() and forward()
    preparedGrep "move\(.+\)" | ${grepCommand} -v "std::move" | ${grepCommand} -E "[^a-z]move"
    preparedGrep "forward\(.+\)" | ${grepCommand} -v "std::forward" | ${grepCommand} -E "[^a-z]forward"
    # make sure `using namespace std` is not used in INCLUDE_DIRECTORIES
    # shellcheck disable=SC2068,SC2068
    ${grepCommand} -nIE -d skip "using namespace std;" ${NAMESPACE_STD_FREE_FILES[@]}
) | ${grepCommand} -E -v -e "^[a-zA-Z\./]*:[0-9]*:[[:space:]]*/(/|\*)" -e "^test/" || true
)

# Special error handling for `using namespace std;` exclusion, since said statement can be present in the test directory
# and its subdirectories, but is excluded in the above ruleset. In order to have consistent codestyle with regards to
# std namespace usage, test directory must also be covered.
FORMATSTDERROR=$(
(
    # make sure `using namespace std` is not used in INCLUDE_DIRECTORIES
    # shellcheck disable=SC2068,SC2068
    grep -nIE -d skip "using namespace std;" ${NAMESPACE_STD_FREE_FILES[@]}
) || true
)

# Merge errors into single string
FORMATEDERRORS="$FORMATERROR$FORMATSTDERROR"

if [[ "$FORMATEDERRORS" != "" ]]
then
    echo "Coding style error:" | tee -a "$ERROR_LOG"
    echo "$FORMATEDERRORS" | tee -a "$ERROR_LOG"
    scripts/ci/post_style_errors_on_github.sh "$ERROR_LOG"
    exit 1
fi
)
