#!/usr/bin/env bash

. scripts/report_errors.sh

(
REPO_ROOT="$(dirname "$0")"/..
cd $REPO_ROOT

WHITESPACE=$(git grep -n -I -E "^.*[[:space:]]+$" | grep -v "test/libsolidity/ASTJSON\|test/compilationTests/zeppelin/LICENSE")

if [[ "$WHITESPACE" != "" ]]
then
	echo "Error: Trailing whitespace found:" | tee -a $ERROR_LOG
	echo "$WHITESPACE" | tee -a $ERROR_LOG
	exit 1
fi

FORMATERROR=$(
(
	git grep -nIE "\<(if|for)\(" -- '*.h' '*.cpp' # no space after "if" or "for"
	git grep -nIE "\<if\>\s*\(.*\)\s*\{\s*$" -- '*.h' '*.cpp' # "{\n" on same line as "if" / "for"
	git grep -nIE "[,\(<]\s*const " -- '*.h' '*.cpp' # const on left side of type
	git grep -nIE "^ [^*]|[^*] 	|	 [^*]" -- '*.h' '*.cpp' # uses spaces for indentation or mixes spaces and tabs
	git grep -nIE "[a-zA-Z0-9_]\s*[&][a-zA-Z_]" -- '*.h' '*.cpp' | egrep -v "return [&]" # right-aligned reference ampersand (needs to exclude return)
	# right-aligned reference pointer star (needs to exclude return and comments)
	git grep -nIE "[a-zA-Z0-9_]\s*[*][a-zA-Z_]" -- '*.h' '*.cpp' | egrep -v "return [*]" | egrep -v "^* [*]" | egrep -v "^*//.*"
) | egrep -v "^[a-zA-Z\./]*:[0-9]*:\s*\/(\/|\*)" | egrep -v "^test/"
)

if [[ "$FORMATERROR" != "" ]]
then
	echo "Coding style error:" | tee -a $ERROR_LOG
	echo "$FORMATERROR" | tee -a $ERROR_LOG
	exit 1
fi
)
