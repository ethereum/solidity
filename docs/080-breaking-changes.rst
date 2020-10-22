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
* Exponentiation is right associative, i.e., the expression ``a**b**c`` is parsed as ``a**(b**c)``.
  Before 0.8.0, it was parsed as ``(a**b)**c``.

Syntactic Only Changes
======================

* The global functions ``log0``, ``log1``, ``log2``, ``log3`` and ``log4`` have been removed.

These are low-level functions that were largely unused. Their behaviour can be accessed from inline assembly.