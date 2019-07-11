# The Solidity to Yul Code Generator

This directory contains the new experimental code generator that
compiles Solidity to an intermediate representation in Yul
with EVM dialect.

The main semantic differences to the legacy code generator are the following:

 - Arithmetic operations cause a failing assertion if the result is not in range.
 - Resizing a storage array to a length larger than 2**64 causes a failing assertion.