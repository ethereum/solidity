********************************
Solidity v0.6.0 Breaking Changes
********************************

This section highlights the main breaking changes introduced in Solidity
version 0.6.0, along with the reasoning behind the changes and how to update
affected code.
For the full list check
`the release changelog <https://github.com/ethereum/solidity/releases/tag/v0.6.0>`_.


Changes the Compiler Might not Warn About
=========================================

This section lists changes where the behavior of your code might
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
the compiler. These may change the way how you use the compiler on the command-line, how you use its programmable
interface, or how you analyze the output produced by it.

New Error Reporter
~~~~~~~~~~~~~~~~~~

A new error reporter was introduced, which aims at producing more accessible error messages on the command-line.
It is enabled by default, but passing ``--old-reporter`` falls back to the deprecated old error reporter.

Metadata Hash Options
~~~~~~~~~~~~~~~~~~~~~

The compiler now appends the `IPFS <https://ipfs.io/>`_ hash of the metadata file to the end of the bytecode by default
(for details, see documentation on :doc:`contract metadata <metadata>`). Before 0.6.0, the compiler appended the
`Swarm <https://ethersphere.github.io/swarm-home/>`_ hash by default, and in order to still support this behavior,
the new command-line option ``--metadata-hash`` was introduced. It allows you to select the hash to be produced and
appended, by passing either ``ipfs`` or ``swarm`` as value to the ``--metadata-hash`` command-line option.
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
