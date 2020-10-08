********************************
Solidity v0.8.0 Breaking Changes
********************************

This section highlights the main breaking changes introduced in Solidity
version 0.8.0, along with the reasoning behind the changes and how to update
affected code.
For the full list check
`the release changelog <https://github.com/ethereum/solidity/releases/tag/v0.8.0>`_.

Semantic and Syntactic Changes
==============================

This section lists changes where you have to modify your code
and it does something else afterwards.

* Explicit conversions from negative literals and literals larger than ``type(uint160).max`` to ``address`` are now disallowed.
