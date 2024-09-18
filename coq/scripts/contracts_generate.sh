#!/bin/sh
set -x #echo on

# Generate various files needed for the verification of the contracts.
#
# We assume that `solc` has been compiled from the source in the `build/` directory at the root of
# the repository. This script should be launched from the `coq/` directory.

# erc20
../build/solc/solc --ir-coq --optimize CoqOfSolidity/contracts/erc20/erc20.sol \
  > CoqOfSolidity/contracts/erc20/erc20.v
../build/solc/solc --ir-optimized-ast-json --optimize CoqOfSolidity/contracts/erc20/erc20.sol \
  | tail -1 \
  | jq 'walk(if type == "object" then del(.nativeSrc, .src, .type) else . end)' \
  > CoqOfSolidity/contracts/erc20/erc20.json
python scripts/shallow_embed.py CoqOfSolidity/contracts/erc20/erc20.json \
  > CoqOfSolidity/contracts/erc20/erc20_shallow.v
python scripts/shallow_embed_proof.py CoqOfSolidity/contracts/erc20/erc20.json \
  > CoqOfSolidity/contracts/erc20/erc20_shallow_proof.v

# SCL_mulmuladdX_fullgen_b4
../build/solc/solc --ir-coq --optimize CoqOfSolidity/contracts/scl/mulmuladdX_fullgen_b4/contract.sol \
  > CoqOfSolidity/contracts/scl/mulmuladdX_fullgen_b4/contract.v
../build/solc/solc --ir-optimized-ast-json --optimize CoqOfSolidity/contracts/scl/mulmuladdX_fullgen_b4/contract.sol \
  | tail -1 \
  | jq 'walk(if type == "object" then del(.nativeSrc, .src, .type) else . end)' \
  > CoqOfSolidity/contracts/scl/mulmuladdX_fullgen_b4/contract.json
python scripts/shallow_embed.py CoqOfSolidity/contracts/scl/mulmuladdX_fullgen_b4/contract.json \
  > CoqOfSolidity/contracts/scl/mulmuladdX_fullgen_b4/contract_shallow.v
python scripts/shallow_embed_proof.py CoqOfSolidity/contracts/scl/mulmuladdX_fullgen_b4/contract.json \
  > CoqOfSolidity/contracts/scl/mulmuladdX_fullgen_b4/contract_shallow_proof.v
