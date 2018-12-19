.. index:: ! contract

.. _contracts:

##########
Contracts
##########

Contracts in Solidity are similar to classes in object-oriented languages. They
contain persistent data in state variables and functions that can modify these
variables. Calling a function on a different contract (instance) will perform
an EVM function call and thus switch the context such that state variables are
inaccessible.

.. include:: conracts/creating-contracts.rst
.. include:: conracts/visibility-and-getters.rst
.. include:: conracts/function-modifiers.rst
.. include:: conracts/constat-state-variables.rst
.. include:: conracts/functions.rst
.. include:: conracts/events.rst
.. include:: conracts/inheritance.rst

.. index:: ! constructor

.. _constructor:

Constructors
============

A constructor is an optional function declared with the ``constructor`` keyword
which is executed upon contract creation, and where you can run contract
initialisation code.

Before the constructor code is executed, state variables are initialised to
their specified value if you initialise them inline, or zero if you do not.

After the constructor has run, the final code of the contract is deployed
to the blockchain. The deployment of
the code costs additional gas linear to the length of the code.
This code includes all functions that are part of the public interface
and all functions that are reachable from there through function calls.
It does not include the constructor code or internal functions that are
only called from the constructor.

Constructor functions can be either ``public`` or ``internal``. If there is no
constructor, the contract will assume the default constructor, which is
equivalent to ``constructor() public {}``. For example:

::

    pragma solidity >0.4.99 <0.6.0;

    contract A {
        uint public a;

        constructor(uint _a) internal {
            a = _a;
        }
    }

    contract B is A(1) {
        constructor() public {}
    }

A constructor set as ``internal`` causes the contract to be marked as :ref:`abstract <abstract-contract>`.

.. warning ::
    Prior to version 0.4.22, constructors were defined as functions with the same name as the contract.
    This syntax was deprecated and is not allowed anymore in version 0.5.0.


.. index:: ! base;constructor

Arguments for Base Constructors
===============================

The constructors of all the base contracts will be called following the
linearization rules explained below. If the base constructors have arguments,
derived contracts need to specify all of them. This can be done in two ways::

    pragma solidity >=0.4.22 <0.6.0;

    contract Base {
        uint x;
        constructor(uint _x) public { x = _x; }
    }

    // Either directly specify in the inheritance list...
    contract Derived1 is Base(7) {
        constructor() public {}
    }

    // or through a "modifier" of the derived constructor.
    contract Derived2 is Base {
        constructor(uint _y) Base(_y * _y) public {}
    }

One way is directly in the inheritance list (``is Base(7)``).  The other is in
the way a modifier is invoked as part of
the derived constructor (``Base(_y * _y)``). The first way to
do it is more convenient if the constructor argument is a
constant and defines the behaviour of the contract or
describes it. The second way has to be used if the
constructor arguments of the base depend on those of the
derived contract. Arguments have to be given either in the
inheritance list or in modifier-style in the derived constructor.
Specifying arguments in both places is an error.

If a derived contract does not specify the arguments to all of its base
contracts' constructors, it will be abstract.

.. index:: ! inheritance;multiple, ! linearization, ! C3 linearization

.. _multi-inheritance:

Multiple Inheritance and Linearization
======================================

Languages that allow multiple inheritance have to deal with
several problems.  One is the `Diamond Problem <https://en.wikipedia.org/wiki/Multiple_inheritance#The_diamond_problem>`_.
Solidity is similar to Python in that it uses "`C3 Linearization <https://en.wikipedia.org/wiki/C3_linearization>`_"
to force a specific order in the directed acyclic graph (DAG) of base classes. This
results in the desirable property of monotonicity but
disallows some inheritance graphs. Especially, the order in
which the base classes are given in the ``is`` directive is
important: You have to list the direct base contracts
in the order from "most base-like" to "most derived".
Note that this order is the reverse of the one used in Python.

Another simplifying way to explain this is that when a function is called that
is defined multiple times in different contracts, the given bases
are searched from right to left (left to right in Python) in a depth-first manner,
stopping at the first match. If a base contract has already been searched, it is skipped.

In the following code, Solidity will give the
error "Linearization of inheritance graph impossible".

::

    pragma solidity >=0.4.0 <0.6.0;

    contract X {}
    contract A is X {}
    // This will not compile
    contract C is A, X {}

The reason for this is that ``C`` requests ``X`` to override ``A``
(by specifying ``A, X`` in this order), but ``A`` itself
requests to override ``X``, which is a contradiction that
cannot be resolved.



Inheriting Different Kinds of Members of the Same Name
======================================================

When the inheritance results in a contract with a function and a modifier of the same name, it is considered as an error.
This error is produced also by an event and a modifier of the same name, and a function and an event of the same name.
As an exception, a state variable getter can override a public function.

.. index:: ! contract;abstract, ! abstract contract

.. _abstract-contract:

******************
Abstract Contracts
******************

Contracts are marked as abstract when at least one of their functions lacks an implementation as in the following example (note that the function declaration header is terminated by ``;``)::

    pragma solidity >=0.4.0 <0.6.0;

    contract Feline {
        function utterance() public returns (bytes32);
    }

Such contracts cannot be compiled (even if they contain implemented functions alongside non-implemented functions), but they can be used as base contracts::

    pragma solidity >=0.4.0 <0.6.0;

    contract Feline {
        function utterance() public returns (bytes32);
    }

    contract Cat is Feline {
        function utterance() public returns (bytes32) { return "miaow"; }
    }

If a contract inherits from an abstract contract and does not implement all non-implemented functions by overriding, it will itself be abstract.

Note that a function without implementation is different from a :ref:`Function Type <function_types>` even though their syntax looks very similar.

Example of function without implementation (a function declaration)::

    function foo(address) external returns (address);

Example of a Function Type (a variable declaration, where the variable is of type ``function``)::

    function(address) external returns (address) foo;

Abstract contracts decouple the definition of a contract from its implementation providing better extensibility and self-documentation and
facilitating patterns like the `Template method <https://en.wikipedia.org/wiki/Template_method_pattern>`_ and removing code duplication.
Abstract contracts are useful in the same way that defining methods in an interface is useful. It is a way for the designer of the abstract contract to say "any child of mine must implement this method".


.. index:: ! contract;interface, ! interface contract

.. _interfaces:

**********
Interfaces
**********

Interfaces are similar to abstract contracts, but they cannot have any functions implemented. There are further restrictions:

- They cannot inherit other contracts or interfaces.
- All declared functions must be external.
- They cannot declare a constructor.
- They cannot declare state variables.

Some of these restrictions might be lifted in the future.

Interfaces are basically limited to what the Contract ABI can represent, and the conversion between the ABI and
an interface should be possible without any information loss.

Interfaces are denoted by their own keyword:

::

    pragma solidity >=0.4.11 <0.6.0;

    interface Token {
        enum TokenType { Fungible, NonFungible }
        struct Coin { string obverse; string reverse; }
        function transfer(address recipient, uint amount) external;
    }

Contracts can inherit interfaces as they would inherit other contracts.

Types defined inside interfaces and other contract-like structures
can be accessed from other contracts: ``Token.TokenType`` or ``Token.Coin``.

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

    pragma solidity >=0.4.22 <0.6.0;

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

    pragma solidity >=0.4.16 <0.6.0;

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

.. index:: ! using for, library

.. _using-for:

*********
Using For
*********

The directive ``using A for B;`` can be used to attach library
functions (from the library ``A``) to any type (``B``).
These functions will receive the object they are called on
as their first parameter (like the ``self`` variable in Python).

The effect of ``using A for *;`` is that the functions from
the library ``A`` are attached to *any* type.

In both situations, *all* functions in the library are attached,
even those where the type of the first parameter does not
match the type of the object. The type is checked at the
point the function is called and function overload
resolution is performed.

The ``using A for B;`` directive is active only within the current
contract, including within all of its functions, and has no effect
outside of the contract in which it is used. The directive
may only be used inside a contract, not inside any of its functions.

By including a library, its data types including library functions are
available without having to add further code.

Let us rewrite the set example from the
:ref:`libraries` in this way::

    pragma solidity >=0.4.16 <0.6.0;

    // This is the same code as before, just without comments
    library Set {
      struct Data { mapping(uint => bool) flags; }

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
        using Set for Set.Data; // this is the crucial change
        Set.Data knownValues;

        function register(uint value) public {
            // Here, all variables of type Set.Data have
            // corresponding member functions.
            // The following function call is identical to
            // `Set.insert(knownValues, value)`
            require(knownValues.insert(value));
        }
    }

It is also possible to extend elementary types in that way::

    pragma solidity >=0.4.16 <0.6.0;

    library Search {
        function indexOf(uint[] storage self, uint value)
            public
            view
            returns (uint)
        {
            for (uint i = 0; i < self.length; i++)
                if (self[i] == value) return i;
            return uint(-1);
        }
    }

    contract C {
        using Search for uint[];
        uint[] data;

        function append(uint value) public {
            data.push(value);
        }

        function replace(uint _old, uint _new) public {
            // This performs the library function call
            uint index = data.indexOf(_old);
            if (index == uint(-1))
                data.push(_new);
            else
                data[index] = _new;
        }
    }

Note that all library calls are actual EVM function calls. This means that
if you pass memory or value types, a copy will be performed, even of the
``self`` variable. The only situation where no copy will be performed
is when storage reference variables are used.
