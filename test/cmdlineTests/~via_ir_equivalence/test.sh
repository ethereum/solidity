#!/usr/bin/env bash
set -eo pipefail

# shellcheck source=scripts/common.sh
source "${REPO_ROOT}/scripts/common.sh"

function test_via_ir_equivalence()
{
    SOLTMPDIR=$(mktemp -d)
    pushd "$SOLTMPDIR" > /dev/null

    (( $# <= 2 )) || fail "This function accepts at most two arguments."
    local solidity_file="$1"
    local optimize_flag="$2"
    [[ $optimize_flag == --optimize || $optimize_flag == "" ]] || assertFail "The second argument must be --optimize if present."

    local output_file_prefix
    output_file_prefix=$(basename "$1" .sol)

    local optimizer_flags=()
    [[ $optimize_flag == "" ]] || optimizer_flags+=("$optimize_flag")
    [[ $optimize_flag == "" ]] || output_file_prefix+="_optimize"

    msg_on_error --no-stderr "$SOLC" --ir-optimized --debug-info location "${optimizer_flags[@]}" "$solidity_file" |
        sed '/^Optimized IR:$/d' |
        split_on_empty_lines_into_numbered_files "$output_file_prefix" ".yul"

    local asm_output_two_stage asm_output_via_ir

    for yul_file in $(find . -name "${output_file_prefix}*.yul" | sort -V); do
        asm_output_two_stage+=$(
            msg_on_error --no-stderr "$SOLC" --strict-assembly --asm "${optimizer_flags[@]}" "$yul_file" |
                sed '/^Text representation:$/d' |
                sed '/^=======/d'
        )
    done

    asm_output_via_ir=$(
        msg_on_error --no-stderr "$SOLC" --via-ir --asm --debug-info location "${optimizer_flags[@]}" "$solidity_file" |
            sed '/^EVM assembly:$/d' |
            sed '/^=======/d'
    )

    diff_values "$asm_output_two_stage" "$asm_output_via_ir" --ignore-space-change --ignore-blank-lines

    local bin_output_two_stage bin_output_via_ir

    for yul_file in $(find . -name "${output_file_prefix}*.yul" | sort -V); do
        bin_output_two_stage+=$(
            msg_on_error --no-stderr "$SOLC" --strict-assembly --bin "${optimizer_flags[@]}" "$yul_file" |
                sed '/^Binary representation:$/d' |
                sed '/^=======/d'
        )
    done

    bin_output_via_ir=$(
        msg_on_error --no-stderr "$SOLC" --via-ir --bin "${optimizer_flags[@]}" "$solidity_file" |
            sed '/^Binary:$/d' |
            sed '/^=======/d'
    )

    diff_values "$bin_output_two_stage" "$bin_output_via_ir" --ignore-space-change --ignore-blank-lines

    popd > /dev/null
    rm -r "$SOLTMPDIR"
}

printTask "Testing the eqivalence of --via-ir and a two-stage compilation..."
(
    externalContracts=(
        externalTests/solc-js/DAO/TokenCreation.sol
        libsolidity/semanticTests/externalContracts/_prbmath/PRBMathSD59x18.sol
        libsolidity/semanticTests/externalContracts/_prbmath/PRBMathUD60x18.sol
        libsolidity/semanticTests/externalContracts/_stringutils/stringutils.sol
        libsolidity/semanticTests/externalContracts/deposit_contract.sol
        libsolidity/semanticTests/externalContracts/FixedFeeRegistrar.sol
        libsolidity/semanticTests/externalContracts/snark.sol
    )

    requiresOptimizer=(
        externalTests/solc-js/DAO/TokenCreation.sol
        libsolidity/semanticTests/externalContracts/deposit_contract.sol
        libsolidity/semanticTests/externalContracts/FixedFeeRegistrar.sol
        libsolidity/semanticTests/externalContracts/snark.sol
    )

    for contractFile in "${externalContracts[@]}"
    do
        if ! [[ "${requiresOptimizer[*]}" =~ $contractFile ]]
        then
            printTask " - ${contractFile}"
            test_via_ir_equivalence "${REPO_ROOT}/test/${contractFile}"
        fi

        printTask " - ${contractFile} (optimized)"
        test_via_ir_equivalence "${REPO_ROOT}/test/${contractFile}" --optimize
    done
)
