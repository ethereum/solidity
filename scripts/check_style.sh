#!/usr/bin/env bash

(
REPO_ROOT="$(dirname "$0")"/..
cd $REPO_ROOT

WHITESPACE=$(git grep -n -I -E "^.*[[:space:]]+$" | grep -v "test/libsolidity/ASTJSON\|test/compilationTests/zeppelin/LICENSE")

if [[ "$WHITESPACE" != "" ]]
then
	echo "Error: Trailing whitespace found:" >&2
	echo "$WHITESPACE" >&2
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
	echo "Error: Format error for if/for:" >&2
	echo "$FORMATERROR" >&2
	exit 1
fi
)
