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

* There are new restrictions on explicit conversions between two different types. The conversion is
  only allowed when there is at most one change in sign, width or 'kind' / different type (``int``,
  ``address``, ``bytesNN``, etc.) For example, the conversion ``uint16(int8)`` is disallowed since
  the conversion changes width (8 bits to 16 bits) and sign (signed integer to unsigned integer.) To
  get the previous behaviour, use an intermediate conversion. In the previous example, this would be
  ``uint16(uint8(int8))`` or ``uint16(int16(int8))``. The following are some examples of conversions
  that are disallowed by this rule. Note that, given types ``T`` and ``S``, the notation ``T(S)``
  refers to the explicit conversion ``T(x)``, where ``x`` is any arbitrary variable of type ``S``.

  - ``address(uint)`` and ``uint(address)``: converting both 'kind' and width. Replace this by
    ``address(uint160(uint))`` and ``uint(uint160(address))`` respectively.
  - ``int80(bytes10)`` and ``bytes10(int80)``: converting both 'kind' and sign. Replace this by
    ``int80(uint80(bytes10))`` and ``bytes10(uint80(int80)`` respectively.
  - ``Contract(uint)``: converting 'kind' and width. Replace this by
    ``Contract(address(uint160(uint)))``.

  These conversions were disallowed since there was ambiguity in such conversions. For example, in
  the expression ``uint16 x = uint16(int8(-1))``, the value of ``x`` would depend on whether the sign or
  the width conversion was applied first.

* Function call options can only be given once, i.e. ``c.f{gas: 10000}{value: 1}()`` is invalid and has to be changed to ``c.f{gas: 10000, value: 1}()``.

* The global functions ``log0``, ``log1``, ``log2``, ``log3`` and ``log4`` have been removed.

  These are low-level functions that were largely unused. Their behaviour can be accessed from inline assembly.

* ``enum`` definitions cannot contain more than 256 members.

  This will make it safe to assume that the underlying type in the ABI is always ``uint8``.

* Declarations with the name ``this``, ``super`` and ``_`` are disallowed, with the exception of
  public functions and events.

Interface Changes
=================

* Changed output of ``--combined-json``. JSON fields ``abi``, ``devdoc``, ``userdoc`` and ``storage-layout`` are sub-objects now. Before 0.8.0 they used to be serialised as strings.
