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

* ABI coder v2 is activated by default.

  You can choose to use the old behaviour using ``pragma abicoder v1;``.
  The pragma ``pragma experimental ABIEncoderV2;`` is still valid, but it is deprecated and has no effect.
  If you want to be explicit, please use ``pragma abicoder v2;`` instead.

  Note that ABI coder v2 supports more types than v1 and performs more sanity checks on the inputs.
  ABI coder v2 makes some function calls more expensive and it can also make contract calls
  revert that did not revert with ABI coder v1 when they contain data that does not conform to the
  parameter types.

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

* If constants are used in array length expressions, previous versions of Solidity would use arbitrary precision
  in all branches of the evaluation tree. Now, if constant variables are used as intermediate expressions,
  their values will be properly rounded in the same way as when they are used in run-time expressions.

* The type ``byte`` has been removed. It was an alias of ``bytes1``.

New Restrictions
================

This section lists changes that might cause existing contracts to not compile anymore.

* There are new restrictions related to explicit conversions of literals. The previous behaviour in
  the following cases was likely ambiguous:

  1. Explicit conversions from negative literals and literals larger than ``type(uint160).max`` to
     ``address`` are disallowed.
  2. Explicit conversions between literals and an integer type ``T`` are only allowed if the literal
     lies between ``type(T).min`` and ``type(T).max``. In particular, replace usages of ``uint(-1)``
     with ``type(uint).max``.
  3. Explicit conversions between literals and enums are only allowed if the literal can
     represent a value in the enum.
  4. Explicit conversions between literals and ``address`` type (e.g. ``address(literal)``) have the
     type ``address`` instead of ``address payable``. One can get a payable address type by using an
     explicit conversion, i.e., ``payable(literal)``.

* :ref:`Address literals<address_literals>` have the type ``address`` instead of ``address
  payable``. They can be converted to ``address payable`` by using an explicit conversion, e.g.
  ``payable(0xdCad3a6d3569DF655070DEd06cb7A1b2Ccd1D3AF)``.

* There are new restrictions on explicit type conversions. The conversion is only allowed when there
  is at most one change in sign, width or type-category (``int``, ``address``, ``bytesNN``, etc.).
  To perform multiple changes, use multiple conversions.

  Let us use the notation ``T(S)`` to denote the explicit conversion ``T(x)``, where, ``T`` and
  ``S`` are types, and ``x`` is any arbitrary variable of type ``S``. An example of such a
  disallowed conversion would be ``uint16(int8)`` since it changes both width (8 bits to 16 bits)
  and sign (signed integer to unsigned integer). In order to do the conversion, one has to go
  through an intermediate type. In the previous example, this would be ``uint16(uint8(int8))`` or
  ``uint16(int16(int8))``. Note that the two ways to convert will produce different results e.g.,
  for ``-1``. The following are some examples of conversions that are disallowed by this rule.

  - ``address(uint)`` and ``uint(address)``: converting both type-category and width. Replace this by
    ``address(uint160(uint))`` and ``uint(uint160(address))`` respectively.
  - ``payable(uint160)``, ``payable(bytes20)`` and ``payable(integer-literal)``: converting both
    type-category and state-mutability. Replace this by ``payable(address(uint160))``,
    ``payable(address(bytes20))`` and ``payable(address(integer-literal))`` respectively. Note that
    ``payable(0)`` is valid and is an exception to the rule.
  - ``int80(bytes10)`` and ``bytes10(int80)``: converting both type-category and sign. Replace this by
    ``int80(uint80(bytes10))`` and ``bytes10(uint80(int80)`` respectively.
  - ``Contract(uint)``: converting both type-category and width. Replace this by
    ``Contract(address(uint160(uint)))``.

  These conversions were disallowed to avoid ambiguity. For example, in the expression ``uint16 x =
  uint16(int8(-1))``, the value of ``x`` would depend on whether the sign or the width conversion
  was applied first.

* Function call options can only be given once, i.e. ``c.f{gas: 10000}{value: 1}()`` is invalid and has to be changed to ``c.f{gas: 10000, value: 1}()``.

* The global functions ``log0``, ``log1``, ``log2``, ``log3`` and ``log4`` have been removed.

  These are low-level functions that were largely unused. Their behaviour can be accessed from inline assembly.

* ``enum`` definitions cannot contain more than 256 members.

  This will make it safe to assume that the underlying type in the ABI is always ``uint8``.

* Declarations with the name ``this``, ``super`` and ``_`` are disallowed, with the exception of
  public functions and events. The exception is to make it possible to declare interfaces of contracts
  implemented in languages other than Solidity that do permit such function names.

* Remove support for the ``\b``, ``\f``, and ``\v`` escape sequences in code.
  They can still be inserted via hexadecimal escapes, e.g. ``\x08``, ``\x0c``, and ``\x0b``, respectively.

* The global variables ``tx.origin`` and ``msg.sender`` have the type ``address`` instead of
  ``address payable``. One can convert them into ``address payable`` by using an explicit
  conversion, i.e., ``payable(tx.origin)`` or ``payable(msg.sender)``.

  This change was done since the compiler cannot determine whether or not these addresses
  are payable or not, so it now requires an explicit conversion to make this requirement visible.

* Explicit conversion into ``address`` type always returns a non-payable ``address`` type. In
  particular, the following explicit conversions have the type ``address`` instead of ``address
  payable``:

  - ``address(u)`` where ``u`` is a variable of type ``uint160``. One can convert ``u``
    into the type ``address payable`` by using two explicit conversions, i.e.,
    ``payable(address(u))``.
  - ``address(b)`` where ``b`` is a variable of type ``bytes20``. One can convert ``b``
    into the type ``address payable`` by using two explicit conversions, i.e.,
    ``payable(address(b))``.
  - ``address(c)`` where ``c`` is a contract. Previously, the return type of this
    conversion depended on whether the contract can receive Ether (either by having a receive
    function or a payable fallback function). The conversion ``payable(c)`` has the type ``address
    payable`` and is only allowed when the contract ``c`` can receive Ether. In general, one can
    always convert ``c`` into the type ``address payable`` by using the following explicit
    conversion: ``payable(address(c))``. Note that ``address(this)`` falls under the same category
    as ``address(c)`` and the same rules apply for it.

* The ``chainid`` builtin in inline assembly is now considered ``view`` instead of ``pure``.

* Unary negation cannot be used on unsigned integers anymore, only on signed integers.

Interface Changes
=================

* The output of ``--combined-json`` has changed: JSON fields ``abi``, ``devdoc``, ``userdoc`` and
  ``storage-layout`` are sub-objects now. Before 0.8.0 they used to be serialised as strings.

* The "legacy AST" has been removed (``--ast-json`` on the commandline interface and ``legacyAST`` for standard JSON).
  Use the "compact AST" (``--ast-compact--json`` resp. ``AST``) as replacement.

* The old error reporter (``--old-reporter``) has been removed.


How to update your code
=======================

- If you rely on wrapping arithmetic, surround each operation with ``unchecked { ... }``.
- Optional: If you use SafeMath or a similar library, change ``x.add(y)`` to ``x + y``, ``x.mul(y)`` to ``x * y`` etc.
- Add ``pragma abicoder v1;`` if you want to stay with the old ABI coder.
- Optionally remove ``pragma experimental ABIEncoderV2`` or ``pragma abicoder v2`` since it is redundant.
- Change ``byte`` to ``bytes1``.
- Add intermediate explicit type conversions if required.
- Combine ``c.f{gas: 10000}{value: 1}()`` to ``c.f{gas: 10000, value: 1}()``.
- Change ``msg.sender.transfer(x)`` to ``payable(msg.sender).transfer(x)`` or use a stored variable of ``address payable`` type.
- Change ``x**y**z`` to ``(x**y)**z``.
- Use inline assembly as a replacement for ``log0``, ..., ``log4``.
- Negate unsigned integers by subtracting them from the maximum value of the type and adding 1 (e.g. ``type(uint256).max - x + 1``, while ensuring that `x` is not zero)
