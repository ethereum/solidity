********************************
Solidity v0.5.0 Breaking Changes
********************************

This section highlights the main breaking changes introduced in Solidity
version 0.5.0, along with the reasoning behind the changes and how to update
affected code.
For the full list check
`the release changelog <https://github.com/ethereum/solidity/releases/tag/v0.5.0>`_.

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
  ``x.call(bytes4(keccak256("f(uint256)"), a, b)`` to
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
  variables.  For example, change ``uint[] x = m_x`` to ``uint[] storage x =
  m_x``, and ``function f(uint[][] x)`` to ``function f(uint[][] memory x)``
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
  opposite padding when converting through ``uint32``.

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

::

    // This will not compile with the current version of the compiler
    pragma solidity ^0.4.25;
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

::

    pragma solidity >=0.5.0 <0.7.0;
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

::

    pragma solidity >=0.5.0 <0.7.0;

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

::

    // This will not compile after 0.6.0
    pragma solidity >=0.5.0 <0.5.99;

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

::

    // This will not compile
    pragma solidity ^0.4.25;

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
            bytes b = new bytes(x);
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

::

    pragma solidity >=0.5.0 <0.7.0;

    contract OtherContract {
        uint x;
        function f(uint y) external {
            x = y;
        }
        receive() payable external {}
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
            require(x > 100);
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

        using address_make_payable for address;
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
            address payable addr = unknownContract.make_payable();
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
    library address_make_payable {
        function make_payable(address x) internal pure returns (address payable) {
            return address(uint160(x));
        }
    }
