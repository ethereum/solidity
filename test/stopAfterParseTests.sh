#!/usr/bin/env bash

set -e

READLINK=readlink
if [[ "$OSTYPE" == "darwin"* ]]; then
	READLINK=greadlink
fi
REPO_ROOT=$(${READLINK} -f "$(dirname "$0")"/..)
SOLIDITY_BUILD_DIR=${SOLIDITY_BUILD_DIR:-${REPO_ROOT}/build}
SOLC=${SOLIDITY_BUILD_DIR}/solc/solc
SPLITSOURCES=${REPO_ROOT}/scripts/splitSources.py

FILETMP=$(mktemp -d)
cd "$FILETMP" || exit 1


function testFile
{
	set +e
	ALLOUTPUT=$($SOLC --combined-json ast --pretty-json "$@" --stop-after parsing 2>&1)
	local RESULT=$?
	set -e
	if test ${RESULT} -ne 0; then
		# solc returned failure. Compilation errors and unimplemented features
		# are okay, everything else is a failed test (segfault)
		if ! echo "$ALLOUTPUT" | grep -e "Unimplemented feature:" -e "Error:" -q; then
			echo -n "Test failed on "
			echo "$@"
			echo "$ALLOUTPUT"
			return 1
		fi
	else
		echo -n .
	fi

	return 0;
}

while read -r file; do
	# NOTE: The command returns the name of the input file if it's not a multi-source file
	OUTPUT="$($SPLITSOURCES "$file")"

	set +e
	testFile "$OUTPUT"
	set -e
	FAILED=$?
	rm -r "${FILETMP:?}"/* 2> /dev/null || true

	if [ $FAILED -eq 1 ]
	then
		echo -n "Failure on "
		echo "$file"
		exit 1
	fi
done < <(find "${REPO_ROOT}/test" -iname "*.sol" -and -not -name "documentation.sol" -and -not -name "boost_filesystem_bug.sol")
echo
