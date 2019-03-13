.. index:: ! inheritance, ! base class, ! contract;base, ! deriving

***********
Inheritance
***********

Solidity supports multiple inheritance including polymorphism.

All function calls are virtual, which means that the most derived function
is called, except when the contract name is explicitly given or the
``super`` keyword is used.

When a contract inherits from other contracts, only a single
contract is created on the blockchain, and the code from all the base contracts
is compiled into the created contract.

The general inheritance system is very similar to
`Python's <https://docs.python.org/3/tutorial/classes.html#inheritance>`_,
especially concerning multiple inheritance, but there are also
some :ref:`differences <multi-inheritance>`.

Details are given in the following example.

::

    pragma solidity >=0.5.0 <0.7.0;

    contract owned {
        constructor() public { owner = msg.sender; }
        address payable owner;
    }

    // Use `is` to derive from another contract. Derived
    // contracts can access all non-private members including
    // internal functions and state variables. These cannot be
    // accessed externally via `this`, though.
    contract mortal is owned {
        function kill() public {
            if (msg.sender == owner) selfdestruct(owner);
        }
    }

    // These abstract contracts are only provided to make the
    // interface known to the compiler. Note the function
    // without body. If a contract does not implement all
    // functions it can only be used as an interface.
    contract Config {
        function lookup(uint id) public returns (address adr);
    }

    contract NameReg {
        function register(bytes32 name) public;
        function unregister() public;
     }

    // Multiple inheritance is possible. Note that `owned` is
    // also a base class of `mortal`, yet there is only a single
    // instance of `owned` (as for virtual inheritance in C++).
    contract named is owned, mortal {
        constructor(bytes32 name) public {
            Config config = Config(0xD5f9D8D94886E70b06E474c3fB14Fd43E2f23970);
            NameReg(config.lookup(1)).register(name);
        }

        // Functions can be overridden by another function with the same name and
        // the same number/types of inputs.  If the overriding function has different
        // types of output parameters, that causes an error.
        // Both local and message-based function calls take these overrides
        // into account.
        function kill() public {
            if (msg.sender == owner) {
                Config config = Config(0xD5f9D8D94886E70b06E474c3fB14Fd43E2f23970);
                NameReg(config.lookup(1)).unregister();
                // It is still possible to call a specific
                // overridden function.
                mortal.kill();
            }
        }
    }

    // If a constructor takes an argument, it needs to be
    // provided in the header (or modifier-invocation-style at
    // the constructor of the derived contract (see below)).
    contract PriceFeed is owned, mortal, named("GoldFeed") {
       function updateInfo(uint newInfo) public {
          if (msg.sender == owner) info = newInfo;
       }

       function get() public view returns(uint r) { return info; }

       uint info;
    }

Note that above, we call ``mortal.kill()`` to "forward" the
destruction request. The way this is done is problematic, as
seen in the following example::

    pragma solidity >=0.4.22 <0.7.0;

    contract owned {
        constructor() public { owner = msg.sender; }
        address payable owner;
    }

    contract mortal is owned {
        function kill() public {
            if (msg.sender == owner) selfdestruct(owner);
        }
    }

    contract Base1 is mortal {
        function kill() public { /* do cleanup 1 */ mortal.kill(); }
    }

    contract Base2 is mortal {
        function kill() public { /* do cleanup 2 */ mortal.kill(); }
    }

    contract Final is Base1, Base2 {
    }

A call to ``Final.kill()`` will call ``Base2.kill`` as the most
derived override, but this function will bypass
``Base1.kill``, basically because it does not even know about
``Base1``.  The way around this is to use ``super``::

    pragma solidity >=0.4.22 <0.7.0;

    contract owned {
        constructor() public { owner = msg.sender; }
        address payable owner;
    }

    contract mortal is owned {
        function kill() public {
            if (msg.sender == owner) selfdestruct(owner);
        }
    }

    contract Base1 is mortal {
        function kill() public { /* do cleanup 1 */ super.kill(); }
    }


    contract Base2 is mortal {
        function kill() public { /* do cleanup 2 */ super.kill(); }
    }

    contract Final is Base1, Base2 {
    }

If ``Base2`` calls a function of ``super``, it does not simply
call this function on one of its base contracts.  Rather, it
calls this function on the next base contract in the final
inheritance graph, so it will call ``Base1.kill()`` (note that
the final inheritance sequence is -- starting with the most
derived contract: Final, Base2, Base1, mortal, owned).
The actual function that is called when using super is
not known in the context of the class where it is used,
although its type is known. This is similar for ordinary
virtual method lookup.

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

    pragma solidity >=0.5.0 <0.7.0;

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

    pragma solidity >=0.4.22 <0.7.0;

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

    pragma solidity >=0.4.0 <0.7.0;

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
