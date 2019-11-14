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
contracts (``L.f()`` if ``L`` is the name of the library). Furthermore,
``internal`` functions of libraries are visible in all contracts, just as
if the library were a base contract. Of course, calls to internal functions
use the internal calling convention, which means that all internal types
can be passed and types :ref:`stored in memory <data-location>` will be passed by reference and not copied.
To realize this in the EVM, code of internal library functions
and all functions called from therein will at compile time be pulled into the calling
contract, and a regular ``JUMP`` call will be used instead of a ``DELEGATECALL``.

.. index:: using for, set

The following example illustrates how to use libraries (but manual method
be sure to check out :ref:`using for <using-for>` for a
more advanced example to implement a set).

::

    pragma solidity >=0.4.22 <0.7.0;


    library Set {
        // We define a new struct datatype that will be used to
        // hold its data in the calling contract.
        struct Data { mapping(uint => bool) flags; }

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
        Set.Data knownValues;

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

    pragma solidity >=0.4.16 <0.7.0;

    library BigInt {
        struct bigint {
            uint[] limbs;
        }

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
                if (a + b < a || (a + b == uint(-1) && carry > 0))
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
        using BigInt for BigInt.bigint;

        function f() public pure {
            BigInt.bigint memory x = BigInt.fromUint(7);
            BigInt.bigint memory y = BigInt.fromUint(uint(-1));
            BigInt.bigint memory z = x.add(y);
            assert(z.limb(1) > 0);
        }
    }

It is possible to obtain the address of a library by converting
the library type to the ``address`` type, i.e. using ``address(LibraryName)``.

As the compiler cannot know where the library will be
deployed at, these addresses have to be filled into the
final bytecode by a linker
(see :ref:`commandline-compiler` for how to use the
commandline compiler for linking). If the addresses are not
given as arguments to the compiler, the compiled hex code
will contain placeholders of the form ``__Set______`` (where
``Set`` is the name of the library). The address can be filled
manually by replacing all those 40 symbols by the hex
encoding of the address of the library contract.

.. note::
    Manually linking libraries on the generated bytecode is discouraged, because
    it is restricted to 36 characters.
    You should ask the compiler to link the libraries at the time
    a contract is compiled by either using
    the ``--libraries`` option of ``solc`` or the ``libraries`` key if you use
    the standard-JSON interface to the compiler.

Restrictions for libraries in comparison to contracts:

- No state variables
- Cannot inherit nor be inherited
- Cannot receive Ether

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
 - Storage pointer types use the type identifier of their corresponding non-storage type, but append a single space
   followed by ``storage`` to it.

The argument encoding is the same as for the regular contract ABI, except for storage pointers, which are encoded as a
``uint256`` value referring to the storage slot to which they point.

Similarly to the contract ABI, the selector consists of the first four bytes of the Keccak256-hash of the signature.
Its value can be obtained from Solidity using the ``.selector`` member as follows:

::

    pragma solidity >0.5.13 <0.7.0;

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
