.. index:: ! library, callcode, delegatecall

.. _libraries:

*********
Libraries
*********

Libraries are similar to contracts, but their purpose is that they are deployed
only once at a specific address and their code is reused using the ``DELEGATECALL``
(``CALLCODE`` until Homestead)
feature of the EVM. This means that if library functions are called, their code
is executed in the context of the calling contract, i.e. ``this`` points to the
calling contract, and especially the storage from the calling contract can be
accessed. As a library is an isolated piece of source code, it can only access
state variables of the calling contract if they are explicitly supplied (it
would have no way to name them, otherwise). Library functions can only be
called directly (i.e. without the use of ``DELEGATECALL``) if they do not modify
the state (i.e. if they are ``view`` or ``pure`` functions),
because libraries are assumed to be stateless. In particular, it is
not possible to destroy a library.

.. note::
    Until version 0.4.20, it was possible to destroy libraries by
    circumventing Solidity's type system. Starting from that version,
    libraries contain a :ref:`mechanism<call-protection>` that
    disallows state-modifying functions
    to be called directly (i.e. without ``DELEGATECALL``).

Libraries can be seen as implicit base contracts of the contracts that use them.
They will not be explicitly visible in the inheritance hierarchy, but calls
to library functions look just like calls to functions of explicit base
contracts (using qualified access like ``L.f()``).
Of course, calls to internal functions
use the internal calling convention, which means that all internal types
can be passed and types :ref:`stored in memory <data-location>` will be passed by reference and not copied.
To realize this in the EVM, code of internal library functions
and all functions called from therein will at compile time be included in the calling
contract, and a regular ``JUMP`` call will be used instead of a ``DELEGATECALL``.

.. index:: using for, set

The following example illustrates how to use libraries (but using a manual method,
be sure to check out :ref:`using for <using-for>` for a
more advanced example to implement a set).

::

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.0 <0.9.0;


    // We define a new struct datatype that will be used to
    // hold its data in the calling contract.
    struct Data {
        mapping(uint => bool) flags;
    }

    library Set {
        // Note that the first parameter is of type "storage
        // reference" and thus only its storage address and not
        // its contents is passed as part of the call.  This is a
        // special feature of library functions.  It is idiomatic
        // to call the first parameter `self`, if the function can
        // be seen as a method of that object.
        function insert(Data storage self, uint value)
            public
            returns (bool)
        {
            if (self.flags[value])
                return false; // already there
            self.flags[value] = true;
            return true;
        }

        function remove(Data storage self, uint value)
            public
            returns (bool)
        {
            if (!self.flags[value])
                return false; // not there
            self.flags[value] = false;
            return true;
        }

        function contains(Data storage self, uint value)
            public
            view
            returns (bool)
        {
            return self.flags[value];
        }
    }


    contract C {
        Data knownValues;

        function register(uint value) public {
            // The library functions can be called without a
            // specific instance of the library, since the
            // "instance" will be the current contract.
            require(Set.insert(knownValues, value));
        }
        // In this contract, we can also directly access knownValues.flags, if we want.
    }

Of course, you do not have to follow this way to use
libraries: they can also be used without defining struct
data types. Functions also work without any storage
reference parameters, and they can have multiple storage reference
parameters and in any position.

The calls to ``Set.contains``, ``Set.insert`` and ``Set.remove``
are all compiled as calls (``DELEGATECALL``) to an external
contract/library. If you use libraries, be aware that an
actual external function call is performed.
``msg.sender``, ``msg.value`` and ``this`` will retain their values
in this call, though (prior to Homestead, because of the use of ``CALLCODE``, ``msg.sender`` and
``msg.value`` changed, though).

The following example shows how to use :ref:`types stored in memory <data-location>` and
internal functions in libraries in order to implement
custom types without the overhead of external function calls:

::

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.8 <0.9.0;

    struct bigint {
        uint[] limbs;
    }

    library BigInt {
        function fromUint(uint x) internal pure returns (bigint memory r) {
            r.limbs = new uint[](1);
            r.limbs[0] = x;
        }

        function add(bigint memory _a, bigint memory _b) internal pure returns (bigint memory r) {
            r.limbs = new uint[](max(_a.limbs.length, _b.limbs.length));
            uint carry = 0;
            for (uint i = 0; i < r.limbs.length; ++i) {
                uint a = limb(_a, i);
                uint b = limb(_b, i);
                r.limbs[i] = a + b + carry;
                if (a + b < a || (a + b == type(uint).max && carry > 0))
                    carry = 1;
                else
                    carry = 0;
            }
            if (carry > 0) {
                // too bad, we have to add a limb
                uint[] memory newLimbs = new uint[](r.limbs.length + 1);
                uint i;
                for (i = 0; i < r.limbs.length; ++i)
                    newLimbs[i] = r.limbs[i];
                newLimbs[i] = carry;
                r.limbs = newLimbs;
            }
        }

        function limb(bigint memory _a, uint _limb) internal pure returns (uint) {
            return _limb < _a.limbs.length ? _a.limbs[_limb] : 0;
        }

        function max(uint a, uint b) private pure returns (uint) {
            return a > b ? a : b;
        }
    }

    contract C {
        using BigInt for bigint;

        function f() public pure {
            bigint memory x = BigInt.fromUint(7);
            bigint memory y = BigInt.fromUint(type(uint).max);
            bigint memory z = x.add(y);
            assert(z.limb(1) > 0);
        }
    }

It is possible to obtain the address of a library by converting
the library type to the ``address`` type, i.e. using ``address(LibraryName)``.

As the compiler does not know the address where the library will be deployed, the compiled hex code
will contain placeholders of the form ``__$30bbc0abd4d6364515865950d3e0d10953$__``. The placeholder
is a 34 character prefix of the hex encoding of the keccak256 hash of the fully qualified library
name, which would be for example ``libraries/bigint.sol:BigInt`` if the library was stored in a file
called ``bigint.sol`` in a ``libraries/`` directory. Such bytecode is incomplete and should not be
deployed. Placeholders need to be replaced with actual addresses. You can do that by either passing
them to the compiler when the library is being compiled or by using the linker to update an already
compiled binary. See :ref:`library-linking` for information on how to use the commandline compiler
for linking.

In comparison to contracts, libraries are restricted in the following ways:

- they cannot have state variables
- they cannot inherit nor be inherited
- they cannot receive Ether
- they cannot be destroyed

(These might be lifted at a later point.)

.. _library-selectors:

Function Signatures and Selectors in Libraries
==============================================

While external calls to public or external library functions are possible, the calling convention for such calls
is considered to be internal to Solidity and not the same as specified for the regular :ref:`contract ABI<ABI>`.
External library functions support more argument types than external contract functions, for example recursive structs
and storage pointers. For that reason, the function signatures used to compute the 4-byte selector are computed
following an internal naming schema and arguments of types not supported in the contract ABI use an internal encoding.

The following identifiers are used for the types in the signatures:

 - Value types, non-storage ``string`` and non-storage ``bytes`` use the same identifiers as in the contract ABI.
 - Non-storage array types follow the same convention as in the contract ABI, i.e. ``<type>[]`` for dynamic arrays and
   ``<type>[M]`` for fixed-size arrays of ``M`` elements.
 - Non-storage structs are referred to by their fully qualified name, i.e. ``C.S`` for ``contract C { struct S { ... } }``.
 - Storage pointer mappings use ``mapping(<keyType> => <valueType>) storage`` where ``<keyType>`` and ``<valueType>`` are
   the identifiers for the key and value types of the mapping, respectively.
 - Other storage pointer types use the type identifier of their corresponding non-storage type, but append a single space
   followed by ``storage`` to it.

The argument encoding is the same as for the regular contract ABI, except for storage pointers, which are encoded as a
``uint256`` value referring to the storage slot to which they point.

Similarly to the contract ABI, the selector consists of the first four bytes of the Keccak256-hash of the signature.
Its value can be obtained from Solidity using the ``.selector`` member as follows:

::

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.5.14 <0.9.0;

    library L {
        function f(uint256) external {}
    }

    contract C {
        function g() public pure returns (bytes4) {
            return L.f.selector;
        }
    }



.. _call-protection:

Call Protection For Libraries
=============================

As mentioned in the introduction, if a library's code is executed
using a ``CALL`` instead of a ``DELEGATECALL`` or ``CALLCODE``,
it will revert unless a ``view`` or ``pure`` function is called.

The EVM does not provide a direct way for a contract to detect
whether it was called using ``CALL`` or not, but a contract
can use the ``ADDRESS`` opcode to find out "where" it is
currently running. The generated code compares this address
to the address used at construction time to determine the mode
of calling.

More specifically, the runtime code of a library always starts
with a push instruction, which is a zero of 20 bytes at
compilation time. When the deploy code runs, this constant
is replaced in memory by the current address and this
modified code is stored in the contract. At runtime,
this causes the deploy time address to be the first
constant to be pushed onto the stack and the dispatcher
code compares the current address against this constant
for any non-view and non-pure function.

This means that the actual code stored on chain for a library
is different from the code reported by the compiler as
``deployedBytecode``.
