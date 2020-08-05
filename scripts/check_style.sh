#!/usr/bin/env bash

. scripts/report_errors.sh

(
REPO_ROOT="$(dirname "$0")"/..
cd $REPO_ROOT

WHITESPACE=$(git grep -n -I -E "^.*[[:space:]]+$" | grep -v "test/libsolidity/ASTJSON\|test/libsolidity/ASTRecoveryTests\|test/compilationTests/zeppelin/LICENSE")

if [[ "$WHITESPACE" != "" ]]
then
	echo "Error: Trailing whitespace found:" | tee -a $ERROR_LOG
	echo "$WHITESPACE" | tee -a $ERROR_LOG
	exit 1
fi

function preparedGrep()
{
	git grep -nIE "$1" -- '*.h' '*.cpp' | grep -v "picosha2.h"
	return $?
}


FORMATERROR=$(
(
	preparedGrep "#include \"" | egrep -v -e "license.h" -e "BuildInfo.h"  # Use include with <> characters
	preparedGrep "\<(if|for|while|switch)\(" # no space after "if", "for", "while" or "switch"
	preparedGrep "\<for\>\s*\([^=]*\>\s:\s.*\)" # no space before range based for-loop
	preparedGrep "\<if\>\s*\(.*\)\s*\{\s*$" # "{\n" on same line as "if" / "for"
	preparedGrep "[,\(<]\s*const " # const on left side of type
	preparedGrep "^\s*(static)?\s*const " # const on left side of type (beginning of line)
	preparedGrep "^ [^*]|[^*] 	|	 [^*]" # uses spaces for indentation or mixes spaces and tabs
	preparedGrep "[a-zA-Z0-9_]\s*[&][a-zA-Z_]" | egrep -v "return [&]" # right-aligned reference ampersand (needs to exclude return)
	# right-aligned reference pointer star (needs to exclude return and comments)
	preparedGrep "[a-zA-Z0-9_]\s*[*][a-zA-Z_]" | egrep -v -e "return [*]" -e "^* [*]" -e "^*//.*"
) | egrep -v -e "^[a-zA-Z\./]*:[0-9]*:\s*\/(\/|\*)" -e "^test/"
)

if [[ "$FORMATERROR" != "" ]]
then
	echo "Coding style error:" | tee -a $ERROR_LOG
	echo "$FORMATERROR" | tee -a $ERROR_LOG
	exit 1
fi
)
