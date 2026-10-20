********************************
Solidity v0.9.0 Breaking Changes
********************************

This section highlights the main breaking changes introduced in Solidity
version 0.9.0.
For the full list check
`the release changelog <https://github.com/argotorg/solidity/releases/tag/v0.9.0>`_.

Silent Changes of the Semantics
===============================

This section lists changes where existing code changes its behavior without
the compiler notifying you about it.


New Restrictions
================


Interface Changes
=================

This section lists changes that are unrelated to the language itself, but that have an effect on the interfaces of
the compiler. These may change the way how you use the compiler on the command-line, how you use its programmable
interface, or how you analyze the output produced by it.

* EVM version ``homestead`` is no longer supported. CLI and JSON interfaces now reject it.


How to update your code
=======================


