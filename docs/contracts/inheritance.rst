.. index:: ! inheritance, ! base class, ! contract;base, ! deriving

***********
Inheritance
***********

Solidity supports multiple inheritance including polymorphism.

All function calls are virtual, which means that the most derived function
is called, except when the contract name is explicitly given or the
``super`` keyword is used.

All functions overriding a base function must specify the ``override`` keyword.
See :ref:`Function Overriding <function-overriding>` for more details.

When a contract inherits from other contracts, only a single
contract is created on the blockchain, and the code from all the base contracts
is compiled into the created contract. This means that all internal calls
to functions of base contracts also just use internal function calls
(``super.f(..)`` will use JUMP and not a message call).

The general inheritance system is very similar to
`Python's <https://docs.python.org/3/tutorial/classes.html#inheritance>`_,
especially concerning multiple inheritance, but there are also
some :ref:`differences <multi-inheritance>`.

Details are given in the following example.

::

    pragma solidity >=0.5.0 <0.7.0;


    contract Owned {
        constructor() public { owner = msg.sender; }
        address payable owner;
    }


    // Use `is` to derive from another contract. Derived
    // contracts can access all non-private members including
    // internal functions and state variables. These cannot be
    // accessed externally via `this`, though.
    contract Mortal is Owned {
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
    contract Named is Owned, Mortal {
        constructor(bytes32 name) public {
            Config config = Config(0xD5f9D8D94886E70b06E474c3fB14Fd43E2f23970);
            NameReg(config.lookup(1)).register(name);
        }

        // Functions can be overridden by another function with the same name and
        // the same number/types of inputs.  If the overriding function has different
        // types of output parameters, that causes an error.
        // Both local and message-based function calls take these overrides
        // into account.
        function kill() public override {
            if (msg.sender == owner) {
                Config config = Config(0xD5f9D8D94886E70b06E474c3fB14Fd43E2f23970);
                NameReg(config.lookup(1)).unregister();
                // It is still possible to call a specific
                // overridden function.
                Mortal.kill();
            }
        }
    }


    // If a constructor takes an argument, it needs to be
    // provided in the header (or modifier-invocation-style at
    // the constructor of the derived contract (see below)).
    contract PriceFeed is Owned, Mortal, Named("GoldFeed") {
        function updateInfo(uint newInfo) public {
            if (msg.sender == owner) info = newInfo;
        }

        function kill() public override (Mortal, Named) { Named.kill(); }
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
        function kill() public override { /* do cleanup 1 */ mortal.kill(); }
    }

    contract Base2 is mortal {
        function kill() public override { /* do cleanup 2 */ mortal.kill(); }
    }

    contract Final is Base1, Base2 {
        function kill() public override(Base1, Base2) { Base2.kill(); }
    }

A call to ``Final.kill()`` will call ``Base2.kill`` because we specify it
explicitly in the final override, but this function will bypass
``Base1.kill``. The way around this is to use ``super``::

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
        function kill() public override { /* do cleanup 1 */ super.kill(); }
    }


    contract Base2 is mortal {
        function kill() public override { /* do cleanup 2 */ super.kill(); }
    }

    contract Final is Base1, Base2 {
        function kill() public override(Base1, Base2) { super.kill(); }
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

.. _function-overriding:

.. index:: ! overriding;function

Function Overriding
===================

Base functions can be overridden by inheriting contracts to change their
behavior. The overriding function must then use the ``override`` keyword in the
function header as shown in this example:

::

    pragma solidity >=0.5.0 <0.7.0;

    contract Base
    {
        function foo() public {}
    }

    contract Middle is Base {}

    contract Inherited is Middle
    {
        function foo() public override {}
    }

For multiple inheritance, the most derived base contracts that define the same
function must be specified explicitly after the ``override`` keyword.
In other words, you have to specify all base contracts that define the same function and have not yet been overridden by another base contract (on some path through the inheritance graph).
Additionally, if a contract inherits the same function from multiple (unrelated)
bases, it has to explicitly override it:

::

    pragma solidity >=0.5.0 <0.7.0;

    contract Base1
    {
        function foo() public {}
    }

    contract Base2
    {
        function foo() public {}
    }

    contract Inherited is Base1, Base2
    {
        // Derives from multiple bases defining foo(), so we must explicitly
        // override it
        function foo() public override(Base1, Base2) {}
    }

A function defined in a common base contract does not have to be explicitly
overridden when used with multiple inheritance:

::

    pragma solidity >=0.5.0 <0.7.0;

    contract A { function f() public pure{} }
    contract B is A {}
    contract C is A {}
    // No explicit override required
    contract D is B, C {}

.. _modifier-overriding:

.. index:: ! overriding;modifier

Modifier Overriding
===================

Function modifiers can override each other. This works in the same way as
function overriding (except that there is no overloading for modifiers). The
``override`` keyword must be used in the overriding contract:

::

    pragma solidity >=0.5.0 <0.7.0;

    contract Base
    {
        modifier foo() {_;}
    }

    contract Inherited is Base
    {
        modifier foo() override {_;}
    }


In case of multiple inheritance, all direct base contracts must be specified
explicitly:

::

    pragma solidity >=0.5.0 <0.7.0;

    contract Base1
    {
        modifier foo() {_;}
    }

    contract Base2
    {
        modifier foo() {_;}
    }

    contract Inherited is Base1, Base2
    {
        modifier foo() override(Base1, Base2) {_;}
    }



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

One area where inheritance linearization is especially important and perhaps not as clear is when there are multiple constructors in the inheritance hierarchy. The constructors will always be executed in the linearized order, regardless of the order in which their arguments are provided in the inheriting contract's constructor.  For example:

::

    pragma solidity >=0.4.0 <0.7.0;

    contract Base1 {
        constructor() public {}
    }

    contract Base2 {
        constructor() public {}
    }

    // Constructors are executed in the following order:
    //  1 - Base1
    //  2 - Base2
    //  3 - Derived1
    contract Derived1 is Base1, Base2 {
        constructor() public Base1() Base2() {}
    }

    // Constructors are executed in the following order:
    //  1 - Base2
    //  2 - Base1
    //  3 - Derived2
    contract Derived2 is Base2, Base1 {
        constructor() public Base2() Base1() {}
    }

    // Constructors are still executed in the following order:
    //  1 - Base2
    //  2 - Base1
    //  3 - Derived3
    contract Derived3 is Base2, Base1 {
        constructor() public Base1() Base2() {}
    }


Inheriting Different Kinds of Members of the Same Name
======================================================

When the inheritance results in a contract with a function and a modifier of the same name, it is considered as an error.
This error is produced also by an event and a modifier of the same name, and a function and an event of the same name.
As an exception, a state variable getter can override a public function.
