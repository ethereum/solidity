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
git grep -nIE "\<(if|for)\(" -- '*.h' '*.cpp'
git grep -nIE "\<if\>\s*\(.*\)\s*\{\s*$" -- '*.h' '*.cpp'
) | egrep -v "^[a-zA-Z\./]*:[0-9]*:\s*\/(\/|\*)" | egrep -v "^test/"
)

if [[ "$FORMATERROR" != "" ]]
then
	echo "Error: Format error for if/for:" | tee -a $ERROR_LOG
	echo "$FORMATERROR" | tee -a $ERROR_LOG
	exit 1
fi
)
