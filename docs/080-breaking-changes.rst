********************************
Solidity v0.8.0 Breaking Changes
********************************

This section highlights the main breaking changes introduced in Solidity
version 0.8.0.
For the full list check
`the release changelog <https://github.com/ethereum/solidity/releases/tag/v0.8.0>`_.

Silent Changes of the Semantics
===============================

This section lists changes where existing code changes its behaviour without
the compiler notifying you about it.

* Arithmetic operations revert on underflow and overflow. You can use ``unchecked { ... }`` to use
  the previous wrapping behaviour.

  Checks for overflow are very common, so we made them the default to increase readability of code,
  even if it comes at a slight increase of gas costs.

* Exponentiation is right associative, i.e., the expression ``a**b**c`` is parsed as ``a**(b**c)``.
  Before 0.8.0, it was parsed as ``(a**b)**c``.

  This is the common way to parse the exponentiation operator.

* Failing assertions and other internal checks like division by zero or arithmetic overflow do
  not use the invalid opcode but instead the revert opcode.
  More specifically, they will use error data equal to a function call to ``Panic(uint256)`` with an error code specific
  to the circumstances.

  This will save gas on errors while it still allows static analysis tools to distinguish
  these situations from a revert on invalid input, like a failing ``require``.

* ABI coder v2 is enabled by default. You can switch back to ABI coder v1 by using
  ``pragma abicoder v1;`` or enable it explicitly sing ``pragma abicoder v2;``.

* If a byte array in storage is accessed whose length is encoded incorrectly, a panic is caused.
  A contract cannot get into this situation unless inline assembly is used to modify the raw representation of storage byte arrays.

New Restrictions
================

* There are new restrictions related to explicit conversion of literals. The previous behaviour in
  the following cases was likely ambiguous:

  1. Explicit conversions from negative literals and literals larger than ``type(uint160).max`` to
     ``address`` are disallowed.
  2. Explicit conversions between literals and an integer type ``T`` are only allowed if the literal
     lies between ``type(T).min`` and ``type(T).max``. In particular, replace usages of ``uint(-1)``
     with ``type(uint).max``.
  3. Explicit conversions between literals and enums are only allowed if the literal can
     represent a value in the enum.

* Function call options can only be given once, i.e. ``c.f{gas: 10000}{value: 1}()`` is invalid and has to be changed to ``c.f{gas: 10000, value: 1}()``.

* The global functions ``log0``, ``log1``, ``log2``, ``log3`` and ``log4`` have been removed.

  These are low-level functions that were largely unused. Their behaviour can be accessed from inline assembly.

* ``enum`` definitions cannot contain more than 256 members.

  This will make it safe to assume that the underlying type in the ABI is always ``uint8``.
