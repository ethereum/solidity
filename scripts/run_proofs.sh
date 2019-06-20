#!/usr/bin/env bash

set -e

REPO_ROOT="$(dirname "$0")"/..

git fetch origin
error=0
for new_proof in $(git diff develop --name-only $REPO_ROOT/test/formal/)
do
	set +e
	echo "Proving $new_proof..."
	output=$(python "$REPO_ROOT/$new_proof")
	result=$?
	set -e

	if [[ "$result" != 0 ]]
	then
		echo "Proof $(basename "$new_proof" ".py") failed: $output."
		error=1
	fi
done

if [[ "error" -eq 0 ]]
then
	echo "All proofs succeeded."
fi

exit $error
