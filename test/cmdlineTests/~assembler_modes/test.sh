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
echo '{}' | msg_on_error --silent "$SOLC" - --yul
echo '{}' | msg_on_error --silent "$SOLC" - --strict-assembly

# Test options above in conjunction with --optimize.
# Using both, --assemble and --optimize should fail.
echo '{}' | "$SOLC" - --assemble --optimize &>/dev/null && fail "solc --assemble --optimize did not fail as expected."
echo '{}' | "$SOLC" - --yul --optimize &>/dev/null && fail "solc --yul --optimize did not fail as expected."

# Test yul and strict assembly output
# Non-empty code results in non-empty binary representation with optimizations turned off,
# while it results in empty binary representation with optimizations turned on.
test_solc_assembly_output "{ let x:u256 := 0:u256 mstore(0, x) }" "{ { let x := 0 mstore(0, x) } }" "--yul"
test_solc_assembly_output "{ let x:u256 := bitnot(7:u256) mstore(0, x) }" "{ { let x := bitnot(7) mstore(0, x) } }" "--yul"
test_solc_assembly_output "{ let t:bool := not(true) if t { mstore(0, 1) } }" "{ { let t:bool := not(true) if t { mstore(0, 1) } } }" "--yul"
test_solc_assembly_output "{ let x := 0 mstore(0, x) }" "{ { let x := 0 mstore(0, x) } }" "--strict-assembly"
test_solc_assembly_output "{ let x := 0 mstore(0, x) }" "{ { } }" "--strict-assembly --optimize"
