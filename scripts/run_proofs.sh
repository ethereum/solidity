#!/usr/bin/env bash

set -e

(
REPO_ROOT="$(dirname "$0")"/..
cd "$REPO_ROOT"

git fetch origin
error=0
for new_proof in $(git diff origin/develop --name-only test/formal/)
do
	set +e
	echo "Proving $new_proof..."
	output=$(python "$new_proof")
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
)
