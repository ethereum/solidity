#!/bin/sh

# The light version of a Makefile for testing, as `coqdep` takes several minutes to run on the
# whole project.

# Display the commands being run.
set -x

#coqc -R . CoqOfSolidity -impredicative-set CoqOfSolidity.v
#coqc -R . CoqOfSolidity -impredicative-set test/libsolidity/semanticTests/various/erc20/ERC20.v
#coqc -R . CoqOfSolidity -impredicative-set simulations/CoqOfSolidity.v
coqc -R . CoqOfSolidity -impredicative-set simulations/erc20.v
