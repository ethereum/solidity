#!/usr/bin/env bash
set -euo pipefail

# shellcheck source=scripts/common.sh
source "${REPO_ROOT}/scripts/common.sh"

function test_solc_assembly_output
{
    local input="${1}"
    local expected="${2}"
    IFS=" " read -r -a solc_args <<< "${3}"

    local expected_object="object \"object\" { code ${expected} }"

    output=$(echo "${input}" | msg_on_error --no-stderr "$SOLC" - "${solc_args[@]}")
    empty=$(echo "$output" | tr '\n' ' ' | tr -s ' ' | sed -ne "/${expected_object}/p")
    if [ -z "$empty" ]
    then
        printError "Incorrect assembly output. Expected: "
        >&2 echo -e "${expected}"
        printError "with arguments ${solc_args[*]}, but got:"
        >&2 echo "${output}"
        fail
    fi
}

echo '{}' | msg_on_error --silent "$SOLC" - --assemble
echo '{}' | msg_on_error --silent "$SOLC" - --strict-assembly

# Test in conjunction with --optimize. Using both, --assemble and --optimize should fail.
echo '{}' | "$SOLC" - --assemble --optimize &>/dev/null && fail "solc --assemble --optimize did not fail as expected."

# Test strict assembly output
# Non-empty code results in non-empty binary representation with optimizations turned off,
# while it results in empty binary representation with optimizations turned on.
test_solc_assembly_output "{ let x := 0 mstore(0, x) }" "{ { let x := 0 mstore(0, x) } }" "--strict-assembly"
test_solc_assembly_output "{ let x := 0 mstore(0, x) }" "{ { } }" "--strict-assembly --optimize"

# Test that --yul triggers an error
set +e
output=$(echo '{}' | "$SOLC" - --yul 2>&1)
failed=$?
expected="Error: The typed Yul dialect formerly accessible via --yul is no longer supported, please use --strict-assembly instead."
set -e
if [[ $output != "${expected}" ]] || (( failed == 0 ))
then
    fail "Incorrect error response to --yul flag: $output"
fi
