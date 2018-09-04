#!/usr/bin/env bash

REPO_ROOT="$(dirname "$0")"/..

(
cd $REPO_ROOT
WHITESPACE=$(git grep -n -I -E "^.*[[:space:]]+$" | grep -v "test/libsolidity/ASTJSON\|test/compilationTests/zeppelin/LICENSE")

if [[ "$WHITESPACE" != "" ]]
then
	echo "Error: Trailing whitespace found:" >&2
	echo "\"$WHITESPACE\"" >&2
	exit 1
else
	exit 0
fi
)
