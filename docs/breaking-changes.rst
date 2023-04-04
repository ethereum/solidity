#########################
Solidity Breaking Changes
#########################

This page highlights the main breaking changes introduced in multiple Solidity versions.

********************************
Solidity v0.8.0 Breaking Changes
********************************

This section highlights the main breaking changes introduced in Solidity
version 0.8.0.
For the full list check
`the 0.8.0 release changelog <https://github.com/ethereum/solidity/releases/tag/v0.8.0>`__.

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

********************************
Solidity v0.7.0 Breaking Changes
********************************

This section highlights the main breaking changes introduced in Solidity
version 0.7.0, along with the reasoning behind the changes and how to update
affected code.
For the full list check
`the 0.7.0 release changelog <https://github.com/ethereum/solidity/releases/tag/v0.7.0>`__.


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

* Visibility (``public`` / ``internal``) is not needed for constructors anymore:
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

********************************
Solidity v0.6.0 Breaking Changes
********************************

This section highlights the main breaking changes introduced in Solidity
version 0.6.0, along with the reasoning behind the changes and how to update
affected code.
For the full list check
`the 0.6.0 release changelog <https://github.com/ethereum/solidity/releases/tag/v0.6.0>`__.


Changes the Compiler Might not Warn About
=========================================

This section lists changes where the behaviour of your code might
change without the compiler telling you about it.

* The resulting type of an exponentiation is the type of the base. It used to be the smallest type
  that can hold both the type of the base and the type of the exponent, as with symmetric
  operations. Additionally, signed types are allowed for the base of the exponentiation.


Explicitness Requirements
=========================

This section lists changes where the code now needs to be more explicit,
but the semantics do not change.
For most of the topics the compiler will provide suggestions.

* Functions can now only be overridden when they are either marked with the
  ``virtual`` keyword or defined in an interface. Functions without
  implementation outside an interface have to be marked ``virtual``.
  When overriding a function or modifier, the new keyword ``override``
  must be used. When overriding a function or modifier defined in multiple
  parallel bases, all bases must be listed in parentheses after the keyword
  like so: ``override(Base1, Base2)``.

* Member-access to ``length`` of arrays is now always read-only, even for storage arrays. It is no
  longer possible to resize storage arrays by assigning a new value to their length. Use ``push()``,
  ``push(value)`` or ``pop()`` instead, or assign a full array, which will of course overwrite the existing content.
  The reason behind this is to prevent storage collisions of gigantic
  storage arrays.

* The new keyword ``abstract`` can be used to mark contracts as abstract. It has to be used
  if a contract does not implement all its functions. Abstract contracts cannot be created using the ``new`` operator,
  and it is not possible to generate bytecode for them during compilation.

* Libraries have to implement all their functions, not only the internal ones.

* The names of variables declared in inline assembly may no longer end in ``_slot`` or ``_offset``.

* Variable declarations in inline assembly may no longer shadow any declaration outside the inline assembly block.
  If the name contains a dot, its prefix up to the dot may not conflict with any declaration outside the inline
  assembly block.

* In inline assembly, opcodes that do not take arguments are now represented as "built-in functions" instead of standalone identifiers. So ``gas`` is now ``gas()``.

* State variable shadowing is now disallowed.  A derived contract can only
  declare a state variable ``x``, if there is no visible state variable with
  the same name in any of its bases.


Semantic and Syntactic Changes
==============================

This section lists changes where you have to modify your code
and it does something else afterwards.

* Conversions from external function types to ``address`` are now disallowed. Instead external
  function types have a member called ``address``, similar to the existing ``selector`` member.

* The function ``push(value)`` for dynamic storage arrays does not return the new length anymore (it returns nothing).

* The unnamed function commonly referred to as "fallback function" was split up into a new
  fallback function that is defined using the ``fallback`` keyword and a receive ether function
  defined using the ``receive`` keyword.

  * If present, the receive ether function is called whenever the call data is empty (whether
    or not ether is received). This function is implicitly ``payable``.

  * The new fallback function is called when no other function matches (if the receive ether
    function does not exist then this includes calls with empty call data).
    You can make this function ``payable`` or not. If it is not ``payable`` then transactions
    not matching any other function which send value will revert. You should only need to
    implement the new fallback function if you are following an upgrade or proxy pattern.


New Features
============

This section lists things that were not possible prior to Solidity 0.6.0
or were more difficult to achieve.

* The :ref:`try/catch statement <try-catch>` allows you to react on failed external calls.
* ``struct`` and ``enum`` types can be declared at file level.
* Array slices can be used for calldata arrays, for example ``abi.decode(msg.data[4:], (uint, uint))``
  is a low-level way to decode the function call payload.
* Natspec supports multiple return parameters in developer documentation, enforcing the same naming check as ``@param``.
* Yul and Inline Assembly have a new statement called ``leave`` that exits the current function.
* Conversions from ``address`` to ``address payable`` are now possible via ``payable(x)``, where
  ``x`` must be of type ``address``.


Interface Changes
=================

This section lists changes that are unrelated to the language itself, but that have an effect on the interfaces of
the compiler. These may change the way how you use the compiler on the command line, how you use its programmable
interface, or how you analyze the output produced by it.

New Error Reporter
~~~~~~~~~~~~~~~~~~

A new error reporter was introduced, which aims at producing more accessible error messages on the command line.
It is enabled by default, but passing ``--old-reporter`` falls back to the the deprecated old error reporter.

Metadata Hash Options
~~~~~~~~~~~~~~~~~~~~~

The compiler now appends the `IPFS <https://ipfs.io/>`_ hash of the metadata file to the end of the bytecode by default
(for details, see documentation on :doc:`contract metadata <metadata>`). Before 0.6.0, the compiler appended the
`Swarm <https://ethersphere.github.io/swarm-home/>`_ hash by default, and in order to still support this behaviour,
the new command line option ``--metadata-hash`` was introduced. It allows you to select the hash to be produced and
appended, by passing either ``ipfs`` or ``swarm`` as value to the ``--metadata-hash`` command line option.
Passing the value ``none`` completely removes the hash.

These changes can also be used via the :ref:`Standard JSON Interface<compiler-api>` and effect the metadata JSON generated by the compiler.

The recommended way to read the metadata is to read the last two bytes to determine the length of the CBOR encoding
and perform a proper decoding on that data block as explained in the :ref:`metadata section<encoding-of-the-metadata-hash-in-the-bytecode>`.

Yul Optimizer
~~~~~~~~~~~~~

Together with the legacy bytecode optimizer, the :doc:`Yul <yul>` optimizer is now enabled by default when you call the compiler
with ``--optimize``. It can be disabled by calling the compiler with ``--no-optimize-yul``.
This mostly affects code that uses ABI coder v2.

C API Changes
~~~~~~~~~~~~~

The client code that uses the C API of ``libsolc`` is now in control of the memory used by the compiler. To make
this change consistent, ``solidity_free`` was renamed to ``solidity_reset``, the functions ``solidity_alloc`` and
``solidity_free`` were added and ``solidity_compile`` now returns a string that must be explicitly freed via
``solidity_free()``.


How to update your code
=======================

This section gives detailed instructions on how to update prior code for every breaking change.

* Change ``address(f)`` to ``f.address`` for ``f`` being of external function type.

* Replace ``function () external [payable] { ... }`` by either ``receive() external payable { ... }``,
  ``fallback() external [payable] { ... }`` or both. Prefer
  using a ``receive`` function only, whenever possible.

* Change ``uint length = array.push(value)`` to ``array.push(value);``. The new length can be
  accessed via ``array.length``.

* Change ``array.length++`` to ``array.push()`` to increase, and use ``pop()`` to decrease
  the length of a storage array.

* For every named return parameter in a function's ``@dev`` documentation define a ``@return``
  entry which contains the parameter's name as the first word. E.g. if you have function ``f()`` defined
  like ``function f() public returns (uint value)`` and a ``@dev`` annotating it, document its return
  parameters like so: ``@return value The return value.``. You can mix named and un-named return parameters
  documentation so long as the notices are in the order they appear in the tuple return type.

* Choose unique identifiers for variable declarations in inline assembly that do not conflict
  with declarations outside the inline assembly block.

* Add ``virtual`` to every non-interface function you intend to override. Add ``virtual``
  to all functions without implementation outside interfaces. For single inheritance, add
  ``override`` to every overriding function. For multiple inheritance, add ``override(A, B, ..)``,
  where you list all contracts that define the overridden function in the parentheses. When
  multiple bases define the same function, the inheriting contract must override all conflicting functions.

* In inline assembly, add ``()`` to all opcodes that do not otherwise accept an argument.
  For example, change ``pc`` to ``pc()``, and ``gas`` to ``gas()``.

********************************
Solidity v0.5.0 Breaking Changes
********************************

This section highlights the main breaking changes introduced in Solidity
version 0.5.0, along with the reasoning behind the changes and how to update
affected code.
For the full list check
`the 0.5.0 release changelog <https://github.com/ethereum/solidity/releases/tag/v0.5.0>`__.

.. note::
   Contracts compiled with Solidity v0.5.0 can still interface with contracts
   and even libraries compiled with older versions without recompiling or
   redeploying them.  Changing the interfaces to include data locations and
   visibility and mutability specifiers suffices. See the
   :ref:`Interoperability With Older Contracts <interoperability>` section below.

Semantic Only Changes
=====================

This section lists the changes that are semantic-only, thus potentially
hiding new and different behavior in existing code.

* Signed right shift now uses proper arithmetic shift, i.e. rounding towards
  negative infinity, instead of rounding towards zero.  Signed and unsigned
  shift will have dedicated opcodes in Constantinople, and are emulated by
  Solidity for the moment.

* The ``continue`` statement in a ``do...while`` loop now jumps to the
  condition, which is the common behavior in such cases. It used to jump to the
  loop body. Thus, if the condition is false, the loop terminates.

* The functions ``.call()``, ``.delegatecall()`` and ``.staticcall()`` do not
  pad anymore when given a single ``bytes`` parameter.

* Pure and view functions are now called using the opcode ``STATICCALL``
  instead of ``CALL`` if the EVM version is Byzantium or later. This
  disallows state changes on the EVM level.

* The ABI encoder now properly pads byte arrays and strings from calldata
  (``msg.data`` and external function parameters) when used in external
  function calls and in ``abi.encode``. For unpadded encoding, use
  ``abi.encodePacked``.

* The ABI decoder reverts in the beginning of functions and in
  ``abi.decode()`` if passed calldata is too short or points out of bounds.
  Note that dirty higher order bits are still simply ignored.

* Forward all available gas with external function calls starting from
  Tangerine Whistle.

Semantic and Syntactic Changes
==============================

This section highlights changes that affect syntax and semantics.

* The functions ``.call()``, ``.delegatecall()``, ``staticcall()``,
  ``keccak256()``, ``sha256()`` and ``ripemd160()`` now accept only a single
  ``bytes`` argument. Moreover, the argument is not padded. This was changed to
  make more explicit and clear how the arguments are concatenated. Change every
  ``.call()`` (and family) to a ``.call("")`` and every ``.call(signature, a,
  b, c)`` to use ``.call(abi.encodeWithSignature(signature, a, b, c))`` (the
  last one only works for value types).  Change every ``keccak256(a, b, c)`` to
  ``keccak256(abi.encodePacked(a, b, c))``. Even though it is not a breaking
  change, it is suggested that developers change
  ``x.call(bytes4(keccak256("f(uint256)")), a, b)`` to
  ``x.call(abi.encodeWithSignature("f(uint256)", a, b))``.

* Functions ``.call()``, ``.delegatecall()`` and ``.staticcall()`` now return
  ``(bool, bytes memory)`` to provide access to the return data.  Change
  ``bool success = otherContract.call("f")`` to ``(bool success, bytes memory
  data) = otherContract.call("f")``.

* Solidity now implements C99-style scoping rules for function local
  variables, that is, variables can only be used after they have been
  declared and only in the same or nested scopes. Variables declared in the
  initialization block of a ``for`` loop are valid at any point inside the
  loop.

Explicitness Requirements
=========================

This section lists changes where the code now needs to be more explicit.
For most of the topics the compiler will provide suggestions.

* Explicit function visibility is now mandatory.  Add ``public`` to every
  function and constructor, and ``external`` to every fallback or interface
  function that does not specify its visibility already.

* Explicit data location for all variables of struct, array or mapping types is
  now mandatory. This is also applied to function parameters and return
  variables.  For example, change ``uint[] x = z`` to ``uint[] storage x =
  z``, and ``function f(uint[][] x)`` to ``function f(uint[][] memory x)``
  where ``memory`` is the data location and might be replaced by ``storage`` or
  ``calldata`` accordingly.  Note that ``external`` functions require
  parameters with a data location of ``calldata``.

* Contract types do not include ``address`` members anymore in
  order to separate the namespaces.  Therefore, it is now necessary to
  explicitly convert values of contract type to addresses before using an
  ``address`` member.  Example: if ``c`` is a contract, change
  ``c.transfer(...)`` to ``address(c).transfer(...)``,
  and ``c.balance`` to ``address(c).balance``.

* Explicit conversions between unrelated contract types are now disallowed. You can only
  convert from a contract type to one of its base or ancestor types. If you are sure that
  a contract is compatible with the contract type you want to convert to, although it does not
  inherit from it, you can work around this by converting to ``address`` first.
  Example: if ``A`` and ``B`` are contract types, ``B`` does not inherit from ``A`` and
  ``b`` is a contract of type ``B``, you can still convert ``b`` to type ``A`` using ``A(address(b))``.
  Note that you still need to watch out for matching payable fallback functions, as explained below.

* The ``address`` type  was split into ``address`` and ``address payable``,
  where only ``address payable`` provides the ``transfer`` function.  An
  ``address payable`` can be directly converted to an ``address``, but the
  other way around is not allowed. Converting ``address`` to ``address
  payable`` is possible via conversion through ``uint160``. If ``c`` is a
  contract, ``address(c)`` results in ``address payable`` only if ``c`` has a
  payable fallback function. If you use the :ref:`withdraw pattern<withdrawal_pattern>`,
  you most likely do not have to change your code because ``transfer``
  is only used on ``msg.sender`` instead of stored addresses and ``msg.sender``
  is an ``address payable``.

* Conversions between ``bytesX`` and ``uintY`` of different size are now
  disallowed due to ``bytesX`` padding on the right and ``uintY`` padding on
  the left which may cause unexpected conversion results.  The size must now be
  adjusted within the type before the conversion.  For example, you can convert
  a ``bytes4`` (4 bytes) to a ``uint64`` (8 bytes) by first converting the
  ``bytes4`` variable to ``bytes8`` and then to ``uint64``. You get the
  opposite padding when converting through ``uint32``. Before v0.5.0 any
  conversion between ``bytesX`` and ``uintY`` would go through ``uint8X``. For
  example ``uint8(bytes3(0x291807))`` would be converted to ``uint8(uint24(bytes3(0x291807)))``
  (the result is ``0x07``).

* Using ``msg.value`` in non-payable functions (or introducing it via a
  modifier) is disallowed as a security feature. Turn the function into
  ``payable`` or create a new internal function for the program logic that
  uses ``msg.value``.

* For clarity reasons, the command line interface now requires ``-`` if the
  standard input is used as source.

Deprecated Elements
===================

This section lists changes that deprecate prior features or syntax.  Note that
many of these changes were already enabled in the experimental mode
``v0.5.0``.

Command Line and JSON Interfaces
--------------------------------

* The command line option ``--formal`` (used to generate Why3 output for
  further formal verification) was deprecated and is now removed.  A new
  formal verification module, the SMTChecker, is enabled via ``pragma
  experimental SMTChecker;``.

* The command line option ``--julia`` was renamed to ``--yul`` due to the
  renaming of the intermediate language ``Julia`` to ``Yul``.

* The ``--clone-bin`` and ``--combined-json clone-bin`` command line options
  were removed.

* Remappings with empty prefix are disallowed.

* The JSON AST fields ``constant`` and ``payable`` were removed. The
  information is now present in the ``stateMutability`` field.

* The JSON AST field ``isConstructor`` of the ``FunctionDefinition``
  node was replaced by a field called ``kind`` which can have the
  value ``"constructor"``, ``"fallback"`` or ``"function"``.

* In unlinked binary hex files, library address placeholders are now
  the first 36 hex characters of the keccak256 hash of the fully qualified
  library name, surrounded by ``$...$``. Previously,
  just the fully qualified library name was used.
  This reduces the chances of collisions, especially when long paths are used.
  Binary files now also contain a list of mappings from these placeholders
  to the fully qualified names.

Constructors
------------

* Constructors must now be defined using the ``constructor`` keyword.

* Calling base constructors without parentheses is now disallowed.

* Specifying base constructor arguments multiple times in the same inheritance
  hierarchy is now disallowed.

* Calling a constructor with arguments but with wrong argument count is now
  disallowed.  If you only want to specify an inheritance relation without
  giving arguments, do not provide parentheses at all.

Functions
---------

* Function ``callcode`` is now disallowed (in favor of ``delegatecall``). It
  is still possible to use it via inline assembly.

* ``suicide`` is now disallowed (in favor of ``selfdestruct``).

* ``sha3`` is now disallowed (in favor of ``keccak256``).

* ``throw`` is now disallowed (in favor of ``revert``, ``require`` and
  ``assert``).

Conversions
-----------

* Explicit and implicit conversions from decimal literals to ``bytesXX`` types
  is now disallowed.

* Explicit and implicit conversions from hex literals to ``bytesXX`` types
  of different size is now disallowed.

Literals and Suffixes
---------------------

* The unit denomination ``years`` is now disallowed due to complications and
  confusions about leap years.

* Trailing dots that are not followed by a number are now disallowed.

* Combining hex numbers with unit denominations (e.g. ``0x1e wei``) is now
  disallowed.

* The prefix ``0X`` for hex numbers is disallowed, only ``0x`` is possible.

Variables
---------

* Declaring empty structs is now disallowed for clarity.

* The ``var`` keyword is now disallowed to favor explicitness.

* Assignments between tuples with different number of components is now
  disallowed.

* Values for constants that are not compile-time constants are disallowed.

* Multi-variable declarations with mismatching number of values are now
  disallowed.

* Uninitialized storage variables are now disallowed.

* Empty tuple components are now disallowed.

* Detecting cyclic dependencies in variables and structs is limited in
  recursion to 256.

* Fixed-size arrays with a length of zero are now disallowed.

Syntax
------

* Using ``constant`` as function state mutability modifier is now disallowed.

* Boolean expressions cannot use arithmetic operations.

* The unary ``+`` operator is now disallowed.

* Literals cannot anymore be used with ``abi.encodePacked`` without prior
  conversion to an explicit type.

* Empty return statements for functions with one or more return values are now
  disallowed.

* The "loose assembly" syntax is now disallowed entirely, that is, jump labels,
  jumps and non-functional instructions cannot be used anymore. Use the new
  ``while``, ``switch`` and ``if`` constructs instead.

* Functions without implementation cannot use modifiers anymore.

* Function types with named return values are now disallowed.

* Single statement variable declarations inside if/while/for bodies that are
  not blocks are now disallowed.

* New keywords: ``calldata`` and ``constructor``.

* New reserved keywords: ``alias``, ``apply``, ``auto``, ``copyof``,
  ``define``, ``immutable``, ``implements``, ``macro``, ``mutable``,
  ``override``, ``partial``, ``promise``, ``reference``, ``sealed``,
  ``sizeof``, ``supports``, ``typedef`` and ``unchecked``.

.. _interoperability:

Interoperability With Older Contracts
=====================================

It is still possible to interface with contracts written for Solidity versions prior to
v0.5.0 (or the other way around) by defining interfaces for them.
Consider you have the following pre-0.5.0 contract already deployed:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.4.25;
    // This will report a warning until version 0.4.25 of the compiler
    // This will not compile after 0.5.0
    contract OldContract {
        function someOldFunction(uint8 a) {
            //...
        }
        function anotherOldFunction() constant returns (bool) {
            //...
        }
        // ...
    }

This will no longer compile with Solidity v0.5.0. However, you can define a compatible interface for it:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.5.0 <0.9.0;
    interface OldContract {
        function someOldFunction(uint8 a) external;
        function anotherOldFunction() external returns (bool);
    }

Note that we did not declare ``anotherOldFunction`` to be ``view``, despite it being declared ``constant`` in the original
contract. This is due to the fact that starting with Solidity v0.5.0 ``staticcall`` is used to call ``view`` functions.
Prior to v0.5.0 the ``constant`` keyword was not enforced, so calling a function declared ``constant`` with ``staticcall``
may still revert, since the ``constant`` function may still attempt to modify storage. Consequently, when defining an
interface for older contracts, you should only use ``view`` in place of ``constant`` in case you are absolutely sure that
the function will work with ``staticcall``.

Given the interface defined above, you can now easily use the already deployed pre-0.5.0 contract:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.5.0 <0.9.0;

    interface OldContract {
        function someOldFunction(uint8 a) external;
        function anotherOldFunction() external returns (bool);
    }

    contract NewContract {
        function doSomething(OldContract a) public returns (bool) {
            a.someOldFunction(0x42);
            return a.anotherOldFunction();
        }
    }

Similarly, pre-0.5.0 libraries can be used by defining the functions of the library without implementation and
supplying the address of the pre-0.5.0 library during linking (see :ref:`commandline-compiler` for how to use the
commandline compiler for linking):

.. code-block:: solidity

    // This will not compile after 0.6.0
    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.5.0;

    library OldLibrary {
        function someFunction(uint8 a) public returns(bool);
    }

    contract NewContract {
        function f(uint8 a) public returns (bool) {
            return OldLibrary.someFunction(a);
        }
    }


Example
=======

The following example shows a contract and its updated version for Solidity
v0.5.0 with some of the changes listed in this section.

Old version:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.4.25;
    // This will not compile after 0.5.0

    contract OtherContract {
        uint x;
        function f(uint y) external {
            x = y;
        }
        function() payable external {}
    }

    contract Old {
        OtherContract other;
        uint myNumber;

        // Function mutability not provided, not an error.
        function someInteger() internal returns (uint) { return 2; }

        // Function visibility not provided, not an error.
        // Function mutability not provided, not an error.
        function f(uint x) returns (bytes) {
            // Var is fine in this version.
            var z = someInteger();
            x += z;
            // Throw is fine in this version.
            if (x > 100)
                throw;
            bytes memory b = new bytes(x);
            y = -3 >> 1;
            // y == -1 (wrong, should be -2)
            do {
                x += 1;
                if (x > 10) continue;
                // 'Continue' causes an infinite loop.
            } while (x < 11);
            // Call returns only a Bool.
            bool success = address(other).call("f");
            if (!success)
                revert();
            else {
                // Local variables could be declared after their use.
                int y;
            }
            return b;
        }

        // No need for an explicit data location for 'arr'
        function g(uint[] arr, bytes8 x, OtherContract otherContract) public {
            otherContract.transfer(1 ether);

            // Since uint32 (4 bytes) is smaller than bytes8 (8 bytes),
            // the first 4 bytes of x will be lost. This might lead to
            // unexpected behavior since bytesX are right padded.
            uint32 y = uint32(x);
            myNumber += y + msg.value;
        }
    }

New version:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.5.0;
    // This will not compile after 0.6.0

    contract OtherContract {
        uint x;
        function f(uint y) external {
            x = y;
        }
        function() payable external {}
    }

    contract New {
        OtherContract other;
        uint myNumber;

        // Function mutability must be specified.
        function someInteger() internal pure returns (uint) { return 2; }

        // Function visibility must be specified.
        // Function mutability must be specified.
        function f(uint x) public returns (bytes memory) {
            // The type must now be explicitly given.
            uint z = someInteger();
            x += z;
            // Throw is now disallowed.
            require(x <= 100);
            int y = -3 >> 1;
            require(y == -2);
            do {
                x += 1;
                if (x > 10) continue;
                // 'Continue' jumps to the condition below.
            } while (x < 11);

            // Call returns (bool, bytes).
            // Data location must be specified.
            (bool success, bytes memory data) = address(other).call("f");
            if (!success)
                revert();
            return data;
        }

        using AddressMakePayable for address;
        // Data location for 'arr' must be specified
        function g(uint[] memory /* arr */, bytes8 x, OtherContract otherContract, address unknownContract) public payable {
            // 'otherContract.transfer' is not provided.
            // Since the code of 'OtherContract' is known and has the fallback
            // function, address(otherContract) has type 'address payable'.
            address(otherContract).transfer(1 ether);

            // 'unknownContract.transfer' is not provided.
            // 'address(unknownContract).transfer' is not provided
            // since 'address(unknownContract)' is not 'address payable'.
            // If the function takes an 'address' which you want to send
            // funds to, you can convert it to 'address payable' via 'uint160'.
            // Note: This is not recommended and the explicit type
            // 'address payable' should be used whenever possible.
            // To increase clarity, we suggest the use of a library for
            // the conversion (provided after the contract in this example).
            address payable addr = unknownContract.makePayable();
            require(addr.send(1 ether));

            // Since uint32 (4 bytes) is smaller than bytes8 (8 bytes),
            // the conversion is not allowed.
            // We need to convert to a common size first:
            bytes4 x4 = bytes4(x); // Padding happens on the right
            uint32 y = uint32(x4); // Conversion is consistent
            // 'msg.value' cannot be used in a 'non-payable' function.
            // We need to make the function payable
            myNumber += y + msg.value;
        }
    }

    // We can define a library for explicitly converting ``address``
    // to ``address payable`` as a workaround.
    library AddressMakePayable {
        function makePayable(address x) internal pure returns (address payable) {
            return address(uint160(x));
        }
    }
