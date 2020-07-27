********************************
Solidity v0.7.0 Breaking Changes
********************************

This section highlights the main breaking changes introduced in Solidity
version 0.7.0, along with the reasoning behind the changes and how to update
affected code.
For the full list check
`the release changelog <https://github.com/ethereum/solidity/releases/tag/v0.7.0>`_.


Silent Changes of the Semantics
===============================

* Exponentiation and shifts of literals by non-literals (e.g. ``1 << x`` or ``2 ** x``)
  will always use either the type ``uint256`` (for non-negative literals) or
  ``int256`` (for negative literals) to perform the operation.
  Previously, the operation was performed in the type of the shift amount / the
  exponent which can be misleading.


Changes to the Syntax
=====================

* In external function and contract creation calls, Ether and gas is now specified using a new syntax:
  ``x.f{gas: 10000, value: 2 ether}(arg1, arg2)``.
  The old syntax -- ``x.f.gas(10000).value(2 ether)(arg1, arg2)`` -- will cause an error.

* The global variable ``now`` is deprecated, ``block.timestamp`` should be used instead.
  The single identifier ``now`` is too generic for a global variable and could give the impression
  that it changes during transaction processing, whereas ``block.timestamp`` correctly
  reflects the fact that it is just a property of the block.

* NatSpec comments on variables are only allowed for public state variables and not
  for local or internal variables.

* The token ``gwei`` is a keyword now (used to specify, e.g. ``2 gwei`` as a number)
  and cannot be used as an identifier.

* String literals now can only contain printable ASCII characters and this also includes a variety of
  escape sequences, such as hexadecimal (``\xff``) and unicode escapes (``\u20ac``).

* Unicode string literals are supported now to accommodate valid UTF-8 sequences. They are identified
  with the ``unicode`` prefix: ``unicode"Hello 😃"``.

* State Mutability: The state mutability of functions can now be restricted during inheritance:
  Functions with default state mutability can be overridden by ``pure`` and ``view`` functions
  while ``view`` functions can be overridden by ``pure`` functions.
  At the same time, public state variables are considered ``view`` and even ``pure``
  if they are constants.



Inline Assembly
---------------

* Disallow ``.`` in user-defined function and variable names in inline assembly.
  It is still valid if you use Solidity in Yul-only mode.

* Slot and offset of storage pointer variable ``x`` are accessed via ``x.slot``
  and ``x.offset`` instead of ``x_slot`` and ``x_offset``.

Removal of Unused or Unsafe Features
====================================

Mappings outside Storage
------------------------

* If a struct or array contains a mapping, it can only be used in storage.
  Previously, mapping members were silently skipped in memory, which
  is confusing and error-prone.

* Assignments to structs or arrays in storage does not work if they contain
  mappings.
  Previously, mappings were silently skipped during the copy operation, which
  is misleading and error-prone.

Functions and Events
--------------------

* Visibility (``public`` / ``external``) is not needed for constructors anymore:
  To prevent a contract from being created, it can be marked ``abstract``.
  This makes the visibility concept for constructors obsolete.

* Type Checker: Disallow ``virtual`` for library functions:
  Since libraries cannot be inherited from, library functions should not be virtual.

* Multiple events with the same name and parameter types in the same
  inheritance hierarchy are disallowed.

* ``using A for B`` only affects the contract it is mentioned in.
  Previously, the effect was inherited. Now, you have to repeat the ``using``
  statement in all derived contracts that make use of the feature.

Expressions
-----------

* Shifts by signed types are disallowed.
  Previously, shifts by negative amounts were allowed, but reverted at runtime.

* The ``finney`` and ``szabo`` denominations are removed.
  They are rarely used and do not make the actual amount readily visible. Instead, explicit
  values like ``1e20`` or the very common ``gwei`` can be used.

Declarations
------------

* The keyword ``var`` cannot be used anymore.
  Previously, this keyword would parse but result in a type error and
  a suggestion about which type to use. Now, it results in a parser error.

Interface Changes
=================

* JSON AST: Mark hex string literals with ``kind: "hexString"``.
* JSON AST: Members with value ``null`` are removed from JSON output.
* NatSpec: Constructors and functions have consistent userdoc output.


How to update your code
=======================

This section gives detailed instructions on how to update prior code for every breaking change.

* Change ``x.f.value(...)()`` to ``x.f{value: ...}()``. Similarly ``(new C).value(...)()`` to
  ``new C{value: ...}()`` and ``x.f.gas(...).value(...)()`` to ``x.f{gas: ..., value: ...}()``.
* Change ``now`` to ``block.timestamp``.
* Change types of right operand in shift operators to unsigned types. For example change ``x >> (256 - y)`` to
  ``x >> uint(256 - y)``.
* Repeat the ``using A for B`` statements in all derived contracts if needed.
* Remove the ``public`` keyword from every constructor.
* Remove the ``internal`` keyword from every constructor and add ``abstract`` to the contract (if not already present).
* Change ``_slot`` and ``_offset`` suffixes in inline assembly to ``.slot`` and ``.offset``, respectively.
